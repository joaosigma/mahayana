#pragma once

#include "logger.hpp"
#include "common/stringUtils.hpp"

#include <libs\squirrel\squirrel.h>

#include <mutex>
#include <atomic>
#include <type_traits>
#include <unordered_map>

namespace HorseRadish { namespace Engine
{
	class Runtime
	{
	public:
		class ClassProxy;

		class FunctionContext
		{
			friend class Runtime;

			int mNumParams = 0;
			HSQUIRRELVM &mSqvm;
			std::string mErrorThrown;

		public:
			enum class ParamType { Null, Bool, Integer, Float, String, ClassInstance };

		public:
			FunctionContext(HSQUIRRELVM &sqvm, bool hasFreeVar = true);

			void throwError(const char * const msg);

			int getNumParams() const;
			ParamType getParamType(const int paramIndex) const;

			bool getParamValue(const int paramIndex, int &value) const;
			bool getParamValue(const int paramIndex, bool &value) const;
			bool getParamValue(const int paramIndex, float &value) const;
			bool getParamValue(const int paramIndex, std::string &value) const;

			template<typename T>
			T getParamValue(const int paramIndex) const
			{
				T value;
				getParamValue(paramIndex, value);
				return value;
			}

			template<typename T>
			T* getParamAsClassInstance(const int paramIndex) const
			{
				if (paramIndex < 0 || paramIndex >= mNumParams)
					return nullptr;

				SQUserPointer userPtr;
				if (!SQ_SUCCEEDED(sq_getinstanceup(mSqvm, paramIndex + 2, &userPtr, 0)))
					return nullptr;

				return reinterpret_cast<T*>(userPtr);
			}

			template<typename T, typename... TValues>
			T* createClassInstance(const char* className, TValues&&... ctorParams)
			{
				static_assert(std::is_base_of<ClassProxy, T>::value, "T must inherit from ClassProxy<T>");

				T* newInstance = nullptr;

				Runtime::pushFullPathToStack(mSqvm, className, false, [&](const std::wstring& lastToken)
				{
					sq_pushstring(mSqvm, lastToken.c_str(), lastToken.size());
					if (!SQ_SUCCEEDED(sq_get(mSqvm, -2)))
						return;

					sq_createinstance(mSqvm, -1);

					HSQOBJECT instanceObj;
					sq_resetobject(&instanceObj);
					sq_getstackobj(mSqvm, -1, &instanceObj);

					sq_addref(mSqvm, &instanceObj);
					newInstance = new T(mSqvm, instanceObj, std::forward<TValues>(ctorParams)...);

					sq_setinstanceup(mSqvm, -1, newInstance);
					sq_setreleasehook(mSqvm, -1, NativeClass<T>::vmDeleteClassInstance);

					sq_pop(mSqvm, 2); //pop the instance and class
				});

				return newInstance;
			}
		};

		class FunctionReturnContext : public FunctionContext
		{
			friend class Runtime;

			bool mReturnValueSet;

		public:
			FunctionReturnContext(HSQUIRRELVM &sqvm);

			void setReturnValue();

			void setReturnValue(const int value);
			void setReturnValue(const bool value);
			void setReturnValue(const float value);
			void setReturnValue(const char * const value);
			void setReturnValue(const std::string &value);
			void setReturnValue(const ClassProxy &value);

			void setReturnValue(const std::vector<int> &values);
			void setReturnValue(const std::vector<bool> &values);
			void setReturnValue(const std::vector<float> &values);
			void setReturnValue(const std::vector<std::string> &values);
		};

		typedef std::function<void(const std::string &, FunctionReturnContext &)> RegisteredFuncType;

		class ClassProxy
		{
			friend class Runtime;

		private:
			HSQUIRRELVM mVM;
			HSQOBJECT mVMInstance;

		public:
			ClassProxy(HSQUIRRELVM vm, HSQOBJECT vmInstance)
				: mVM(vm), mVMInstance(vmInstance)
			{ }

			template<typename... TValues>
			bool invokeVoidMethod(const char* const funcName, TValues&&... funcParams)
			{
				sq_pushobject(mVM, mVMInstance);

				int objsOnStack = 0;
				bool methodFound = false;

				Runtime::parsePath(funcName, [&](const std::string& token, bool isLastToken) -> bool
				{
					auto tokenWChar = HorseRadish::StringUtils::conv2UTF16(token);

					if (isLastToken)
					{
						sq_pushstring(mVM, tokenWChar.c_str(), tokenWChar.size());
						if (SQ_FAILED(sq_get(mVM, -2)))
							return false;

						objsOnStack++;

						if (sq_gettype(mVM, -1) != OT_CLOSURE)
							return false;

						methodFound = true;
						return false;
					}

					sq_pushstring(mVM, tokenWChar.c_str(), -1);
					if (SQ_FAILED(sq_get(mVM, -2)))
						return false;

					objsOnStack++;
					return true;
				});

				if (methodFound)
				{
					sq_pushobject(mVM, mVMInstance);
					Runtime::vmPushStack(mVM, std::forward<TValues>(funcParams)...);

					sq_call(mVM, sizeof...(funcParams) + 1, SQFalse, SQTrue);
				}

				sq_pop(mVM, objsOnStack + 1); //plus the class instance

				return methodFound;
			}

			void refIncrement();
			bool refDecrement();

			void bindTable(const std::string& varName);
			void bindValue(const std::string& varName, const int& value);
			void bindValue(const std::string& varName, const bool& value);
			void bindValue(const std::string& varName, const float& value);
			void bindValue(const std::string& varName, const char * const value);
			void bindValue(const std::string& varName, const std::string& value);
		};

		template<class T>
		class NativeClass
		{
			friend class Runtime;

		public:
			typedef std::function<void(T &, const std::string &, FunctionReturnContext &)> RegisteredClassFuncType;

		private:
			static std::unordered_map<std::string, RegisteredClassFuncType> sFuncMap;

		private:
			HSQUIRRELVM mVM;
			
		private:
			static SQInteger vmCreateClassInstance(HSQUIRRELVM vm)
			{
				if (sq_gettype(vm, 1) != OT_INSTANCE)
					return 0;

				HSQOBJECT instanceObj;
				sq_resetobject(&instanceObj);
				sq_getstackobj(vm, 1, &instanceObj);

				FunctionContext ctx(vm, false);

				auto pClass = new T(vm, instanceObj, ctx);

				if (!ctx.mErrorThrown.empty())
					return sq_throwerror(vm, HorseRadish::StringUtils::conv2UTF16(ctx.mErrorThrown).c_str());

				sq_setinstanceup(vm, 1, pClass);
				sq_setreleasehook(vm, 1, NativeClass<T>::vmDeleteClassInstance);

				return 0;
			}

			static SQInteger vmDeleteClassInstance(SQUserPointer ptr, SQInteger size)
			{
				delete (reinterpret_cast<T*>(ptr));
				return 0;
			}

			static SQInteger vmRegisteredFunc(HSQUIRRELVM vm)
			{
				const SQChar *funcName;
				if (!SQ_SUCCEEDED(sq_getstring(vm, -1, &funcName)))
					return sq_throwerror(vm, _SC("Internal error - func name not found"));

				auto utf8FuncName = HorseRadish::StringUtils::conv2UTF8(funcName);

				auto funcTarget = NativeClass<T>::sFuncMap[utf8FuncName];
				if (!funcTarget)
					return sq_throwerror(vm, _SC("Internal error - func id invalid"));

				T* pClass;
				if (!SQ_SUCCEEDED(sq_getinstanceup(vm, 1, reinterpret_cast<SQUserPointer*>(&pClass), 0)))
					return sq_throwerror(vm, _SC("Internal error - native class not present"));

				FunctionReturnContext ctx(vm);

				funcTarget(*pClass, utf8FuncName, ctx);
				if (ctx.mErrorThrown.empty())
					return (ctx.mReturnValueSet ? 1 : 0);

				if (ctx.mReturnValueSet)
					sq_pop(vm, 1);

				return sq_throwerror(vm, HorseRadish::StringUtils::conv2UTF16(ctx.mErrorThrown).c_str());
			}

		public:
			NativeClass(HSQUIRRELVM vm)
				: mVM(vm)
			{ }

			bool registerClassMethod(const char* const funcName, RegisteredClassFuncType funcCallback)
			{
				std::string funcNameStr(funcName);
				std::wstring funcNameWChar = HorseRadish::StringUtils::conv2UTF16(funcNameStr);

				sq_pushstring(mVM, funcNameWChar.c_str(), funcNameWChar.size());
					sq_pushstring(mVM, funcNameWChar.c_str(), funcNameWChar.size()); //free var
				sq_newclosure(mVM, NativeClass<T>::vmRegisteredFunc, 1);
				if (!SQ_SUCCEEDED(sq_newslot(mVM, -3, SQFalse)))
					return false;

				NativeClass<T>::sFuncMap[funcNameStr] = funcCallback;
				return true;
			}

			bool registerClassTable(const char* const tableName)
			{
				auto tableNameWChar = HorseRadish::StringUtils::conv2UTF16(tableName);

				sq_pushstring(mVM, tableNameWChar.c_str(), tableNameWChar.size());
				sq_newtable(mVM);
				if (!SQ_SUCCEEDED(sq_newslot(mVM, -3, SQFalse)))
					return false;

				return true;
			}

			bool registerClassVar(const char* const varName)
			{
				auto varNameWChar = HorseRadish::StringUtils::conv2UTF16(varName);

				sq_pushstring(mVM, varNameWChar.c_str(), varNameWChar.size());
				sq_pushnull(mVM);
				if (!SQ_SUCCEEDED(sq_newslot(mVM, -3, SQFalse)))
					return false;

				return true;
			}
		};

	private:
		Logger::Context &mLogger;
		HSQUIRRELVM mVM;
		std::unordered_map<std::string, RegisteredFuncType> mFuncMap;

	private:
		static void parsePath(const char * const fullPath, std::function<bool(const std::string&, bool)> cbTokenFound);
		static bool pushFullPathToStack(HSQUIRRELVM vm, const char * const fullPath, bool checkPathExists, std::function<void(const std::wstring&)> cbOnPathEnd);

		static void squirrelStackTrace(HSQUIRRELVM vm);
		static SQInteger squirrelRuntimeError(HSQUIRRELVM vm);
		static void squirrelCompileError(HSQUIRRELVM vm, const SQChar* description, const SQChar* file, SQInteger line, SQInteger column);
		static void squirrelErrorFunction(HSQUIRRELVM vm, const SQChar *format, ...);
		static void squirrelPrintFunction(HSQUIRRELVM vm, const SQChar *format, ...);

		static SQInteger vmRegisteredFunc(HSQUIRRELVM sqvm);
		static void vmPrintLastError(HSQUIRRELVM sqvm);

		static void vmPushStackValue(HSQUIRRELVM sqvm, const int value);
		static void vmPushStackValue(HSQUIRRELVM sqvm, const bool value);
		static void vmPushStackValue(HSQUIRRELVM sqvm, const float value);
		static void vmPushStackValue(HSQUIRRELVM sqvm, const char * const value);
		static void vmPushStackValue(HSQUIRRELVM sqvm, const std::string &value);

		template<typename T>
		static void vmPushStackValue(HSQUIRRELVM sqvm, const std::vector<T> &values)
		{
			sq_newarray(mSqvm, values.size());

			int curIndex = 0;
			for (const auto& value : values)
			{
				sq_pushinteger(sqvm, curIndex);
				Runtime::vmPushStack(value);
				sq_set(sqvm, -3);

				curIndex++;
			}
		}

		static void vmPushStack(HSQUIRRELVM sqvm)
		{ }

		template<typename TFirst, typename... TRest>
		static void vmPushStack(HSQUIRRELVM sqvm, const TFirst &valueFirst, const TRest&... valuesRest)
		{
			Runtime::vmPushStackValue(sqvm, valueFirst);
			Runtime::vmPushStack(sqvm, valuesRest...);
		}

	public:
		Runtime(Logger::Context &logger);
		~Runtime();

		template<typename... TValues>
		bool callVoidMethod(const char* const funcName, TValues&&... funcParams)
		{
			auto numParams = sizeof...(funcParams);

			return Runtime::pushFullPathToStack(mVM, funcName, false, [&](const std::wstring& lastToken)
			{
				sq_pushstring(mVM, lastToken.c_str(), lastToken.size());
				if (SQ_FAILED(sq_get(mVM, -2)))
					return;

				if (sq_gettype(mVM, -1) != OT_CLOSURE)
				{
					sq_pop(mVM, 2);
					return;
				}

				sq_pushroottable(mVM);
				Runtime::vmPushStack(mVM, std::forward<TValues>(funcParams)...);

				sq_call(mVM, numParams + 1, SQFalse, SQTrue);
				sq_pop(mVM, 1);
			});
		}

		bool registerFunc(const char* const funcName, RegisteredFuncType funcCallback);
		void unregisterFunc(const char* const funcName);

		template <class T>
		void registerClass(const char* const className)
		{
			static_assert(std::is_base_of<ClassProxy, T>::value, "T must inherit from ClassProxy<T>");

			Runtime::pushFullPathToStack(mVM, className, false, [&](const std::wstring& lastToken)
			{
				NativeClass<T> nativeClass(mVM);

				sq_pushstring(mVM, lastToken.c_str(), lastToken.size());
				sq_newclass(mVM, false);

				sq_pushstring(mVM, _SC("constructor"), -1);
				sq_newclosure(mVM, NativeClass<T>::vmCreateClassInstance, 0);
				sq_newslot(mVM, -3, false); //ctor

				T::runtimeRegisterData(nativeClass);

				sq_newslot(mVM, -3, SQFalse); //the class itself
			});
		}

		bool runScript(const char* const script);
		bool runScriptFile(const char* const filePath);
	};

	template <class T>
	std::unordered_map<std::string, typename HorseRadish::Engine::Runtime::NativeClass<T>::RegisteredClassFuncType> HorseRadish::Engine::Runtime::NativeClass<T>::sFuncMap;
} }

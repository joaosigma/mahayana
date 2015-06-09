#pragma once

#include "runtime.hpp"
#include "logger.hpp"

#include "common/Timer.hpp"
#include "common/Types.hpp"
#include "common/AVL-tree.hpp"
#include "common/FileSystem.hpp"

#include "platform/window.hpp"

#include <array>
#include <memory>
#include <string>

namespace HorseRadish { namespace Engine {

	class Engine
	{
	public:
		enum class State {
			Created, Initializing, Running, Stopping, Stopped, StoppedError
		};

		enum class ExitAction {
			Nothing, Restart
		};

		enum class StatSampleType {
			Fps, NumTris
		};

		class IVariable
		{
		public:
			enum class VariableType {
				Bool, Integer, Double, String
			};
		protected:
			VariableType varType;
			std::string desc;

		public:
			IVariable(const VariableType varType)
				: varType(varType)
			{}

			IVariable(const VariableType varType, const std::string &desc)
				: varType(varType), desc(desc)
			{}

			IVariable(const IVariable&) = delete;
			IVariable& operator=(const IVariable&) = delete;

			bool IsOfType(VariableType varType) const
			{
				return (this->varType == varType);
			}

			VariableType GetVarType() const
			{
				return varType;
			}

			const std::string& GetDesc() const
			{
				return desc;
			}
		};

		class VariableBool : public IVariable
		{
			bool value;

		public:
			VariableBool(const bool defaultValue)
				: IVariable(IVariable::VariableType::Bool), value(defaultValue)
			{}

			VariableBool(const bool defaultValue, const std::string& desc)
				: IVariable(IVariable::VariableType::Bool, desc), value(defaultValue)
			{}

			VariableBool(const VariableBool&) = delete;
			VariableBool& operator=(const VariableBool&) = delete;

			bool GetValue() const
			{
				return value;
			}

			void SetValue(const bool newValue)
			{
				value = newValue;
			}
		};

		class VariableInt : public IVariable
		{
			hInt64 value;
			hInt64 clampMin, clampMax;
			bool clampValue;

		public:
			VariableInt(const hInt64 defaultValue)
				: IVariable(IVariable::VariableType::Integer), value(defaultValue), clampMin(0), clampMax(0), clampValue(false)
			{}

			VariableInt(const hInt64 defaultValue, const std::string& desc)
				: IVariable(IVariable::VariableType::Integer, desc), value(defaultValue), clampMin(0), clampMax(0), clampValue(false)
			{}

			VariableInt(const hInt64 defaultValue, const hInt64 clampMinValue, const hInt64 clampMaxValue)
				: IVariable(IVariable::VariableType::Integer), value(defaultValue), clampMin(clampMinValue), clampMax(clampMaxValue), clampValue(true)
			{}

			VariableInt(const hInt64 defaultValue, const hInt64 clampMinValue, const hInt64 clampMaxValue, const std::string& desc)
				: IVariable(IVariable::VariableType::Integer, desc), value(defaultValue), clampMin(clampMinValue), clampMax(clampMaxValue), clampValue(true)
			{}

			VariableInt(const VariableInt&) = delete;
			VariableInt& operator=(const VariableInt&) = delete;

			hInt64 GetValue() const
			{
				return value;
			}

			void SetValue(const hInt64 newValue)
			{
				value = newValue;
				if (clampValue)
					value = value < clampMin ? clampMin : (value > clampMax ? clampMax : value);
			}
		};

		class VariableDouble : public IVariable
		{
			double value;
			double clampMin, clampMax;
			bool clampValue;

		public:
			VariableDouble(const double defaultValue)
				: IVariable(IVariable::VariableType::Double), value(defaultValue), clampMin(0), clampMax(0), clampValue(false)
			{}

			VariableDouble(const double defaultValue, const std::string& desc)
				: IVariable(IVariable::VariableType::Double, desc), value(defaultValue), clampMin(0), clampMax(0), clampValue(false)
			{}

			VariableDouble(const double defaultValue, const double clampMinValue, const double clampMaxValue)
				: IVariable(IVariable::VariableType::Double), value(defaultValue), clampMin(clampMinValue), clampMax(clampMaxValue), clampValue(true)
			{}

			VariableDouble(const double defaultValue, const double clampMinValue, const double clampMaxValue, const std::string& desc)
				: IVariable(IVariable::VariableType::Double, desc), value(defaultValue), clampMin(clampMinValue), clampMax(clampMaxValue), clampValue(true)
			{}

			VariableDouble(const VariableDouble&) = delete;
			VariableDouble& operator=(const VariableDouble&) = delete;

			double GetValue() const
			{
				return value;
			}

			void SetValue(const double newValue)
			{
				value = newValue;
				if (clampValue)
					value = value < clampMin ? clampMin : (value > clampMax ? clampMax : value);
			}
		};

		class VariableString : public IVariable
		{
			std::string value;

		public:
			VariableString(const std::string& defaultValue)
				: IVariable(IVariable::VariableType::String), value(defaultValue)
			{}

			VariableString(const std::string& defaultValue, const std::string& desc)
				: IVariable(IVariable::VariableType::String, desc), value(defaultValue)
			{}

			VariableString(const VariableString&) = delete;
			VariableString& operator=(const VariableString&) = delete;

			const std::string& GetValue() const
			{
				return value;
			}

			void SetValue(const std::string& newValue)
			{
				value = newValue;
			}
		};

	private:
		template<std::size_t N>
		class StatSeries
		{
			double mSampleAccum;
			unsigned int mNumSamples;

			unsigned int mTotalHistory;
			unsigned int mIndexHistory;
			std::array<float, N> mHistory;

			std::string mUnits;

		public:
			StatSeries(const std::string &units)
				: mSampleAccum(0.0), mNumSamples(0), mTotalHistory(0), mIndexHistory(0), mUnits(units)
			{ }

			StatSeries(const StatSeries&) = delete;
			StatSeries& operator=(const StatSeries&) = delete;

			void AccumSample(const float value)
			{
				mSampleAccum += value;
				mNumSamples++;
			}

			void MoveNextSample()
			{
				if (mNumSamples == 0)
					mHistory[mIndexHistory] = 0.0f;
				else if (mNumSamples == 1)
					mHistory[mIndexHistory] = mSampleAccum;
				else
					mHistory[mIndexHistory] = static_cast<float>(mSampleAccum / static_cast<double>(mNumSamples));

				mNumSamples = 0;
				mSampleAccum = 0.0;

				mTotalHistory++;
				mIndexHistory++;
				if (mIndexHistory >= N)
					mIndexHistory = 0;
			}

			float GetLastSample() const
			{
				if (mTotalHistory <= 0)
					return 0.0f;

				return ((mIndexHistory == 0) ? mHistory[N - 1] : mHistory[mIndexHistory - 1]);
			}

			const std::string& GetUnits() const
			{
				return mUnits;
			}
		};

	private:
		mutable std::mutex mSyncLock;

		bool mDevMode;
		std::string mErrorDesc;
		std::atomic<State> mCurState;
		HorseRadish::Timer mMainTimer;
		int mExitCode;
		std::atomic<ExitAction> mExitAction;
		AVLTree<char, std::shared_ptr<IVariable>> mVars;
		std::unordered_map<StatSampleType, std::shared_ptr<StatSeries<3000>>> mStats;

		std::shared_ptr<Logger> mLogger;
		std::shared_ptr<Logger::Context> mLoggerRenderCtx;
		std::shared_ptr<Logger::Context> mLoggerRuntimeCtx;

		std::shared_ptr<Window> mWindow;
		std::shared_ptr<Runtime> mRuntime;
		std::shared_ptr<IO::FileSystem> mFileSystem;

	private:
		void exit(ExitAction exitAction, const char * const errorDesc = nullptr);

		void initFileSystem();

		void renderLoop();

		void runtimeFuncVarCreate(const std::string &funcName, Runtime::FunctionReturnContext &ctx);
		void runtimeFuncVarGet(const std::string &funcName, Runtime::FunctionReturnContext &ctx);
		void runtimeFuncVarSet(const std::string &funcName, Runtime::FunctionReturnContext &ctx);
		void runtimeFuncVarList(const std::string &funcName, Runtime::FunctionReturnContext &ctx);
		void runtimeFuncRuntime(const std::string &funcName, Runtime::FunctionReturnContext &ctx);

		void logSysInfo();

	public:
		Engine(const std::string &cmdLine, bool devMode = false);
		~Engine();

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		// vars
		template<typename T>
		T VarGet(const char* const name) const;

		template<typename T>
		void VarSet(const char* const name, const T& value);

		std::string VarGetFormatted(const char* const name) const;

		// misc
		bool MainLoop();

		int GetExitCode() const;
		ExitAction GetExitAction() const;

		std::string GetErrorDesc() const;
	};
} }

template<> bool HorseRadish::Engine::Engine::VarGet<bool>(const char* const name) const;
template<> int HorseRadish::Engine::Engine::VarGet<int>(const char* const name) const;
template<> HorseRadish::hInt64 HorseRadish::Engine::Engine::VarGet<HorseRadish::hInt64>(const char* const name) const;
template<> float HorseRadish::Engine::Engine::VarGet<float>(const char* const name) const;
template<> double HorseRadish::Engine::Engine::VarGet<double>(const char* const name) const;
template<> std::string HorseRadish::Engine::Engine::VarGet<std::string>(const char* const name) const;

template<> void HorseRadish::Engine::Engine::VarSet(const char* const name, const bool& value);
template<> void HorseRadish::Engine::Engine::VarSet(const char* const name, const int& value);
template<> void HorseRadish::Engine::Engine::VarSet(const char* const name, const HorseRadish::hInt64& value);
template<> void HorseRadish::Engine::Engine::VarSet(const char* const name, const float& value);
template<> void HorseRadish::Engine::Engine::VarSet(const char* const name, const double& value);
template<> void HorseRadish::Engine::Engine::VarSet(const char* const name, const std::string& value);
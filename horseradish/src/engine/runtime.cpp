module;

#include <cstdarg>
#include <stdio.h>

#include <libs/squirrel/squirrel.h>

#include <libs/squirrel/sqstdblob.h>
#include <libs/squirrel/sqstdmath.h>
#include <libs/squirrel/sqstdstring.h>

module Runtime;

import std;

import core;

namespace hr::engine
{
    Runtime::FunctionContext::FunctionContext(HSQUIRRELVM& sqvm, bool hasFreeVar)
      : mSqvm(sqvm)
    {
        mNumParams = sq_gettop(sqvm) - (hasFreeVar ? 2 : 1); //'this' (at 1) and the free variable (at the end of the stack) don't count
    }

    void Runtime::FunctionContext::throwError(const char* const msg)
    {
        mErrorThrown = msg;
    }

    int Runtime::FunctionContext::getNumParams() const
    {
        return mNumParams;
    }

    Runtime::FunctionContext::ParamType Runtime::FunctionContext::getParamType(const int paramIndex) const
    {
        if (paramIndex < 0 || paramIndex >= mNumParams)
            return Runtime::FunctionContext::ParamType::Null;

        switch (sq_gettype(mSqvm, paramIndex + 2))
        {
            case OT_BOOL:
                return Runtime::FunctionContext::ParamType::Bool;
            case OT_INTEGER:
                return Runtime::FunctionContext::ParamType::Integer;
            case OT_FLOAT:
                return Runtime::FunctionContext::ParamType::Float;
            case OT_STRING:
                return Runtime::FunctionContext::ParamType::String;
            case OT_INSTANCE:
                return Runtime::FunctionContext::ParamType::ClassInstance;
            default:
                break;
        }

        return Runtime::FunctionContext::ParamType::Null;
    }

    bool Runtime::FunctionContext::getParamValue(const int paramIndex, int& value) const
    {
        if (paramIndex < 0 || paramIndex >= mNumParams)
            return false;

        SQInteger sqInt = 0; // in x64, SQInteger is not compatible with int
        auto success = SQ_SUCCEEDED(sq_getinteger(mSqvm, paramIndex + 2, &sqInt));

        value = sqInt;
        return success;
    }

    bool Runtime::FunctionContext::getParamValue(const int paramIndex, bool& value) const
    {
        if (paramIndex < 0 || paramIndex >= mNumParams)
            return false;

        SQBool sqVal;
        if (!SQ_SUCCEEDED(sq_getbool(mSqvm, paramIndex + 2, &sqVal)))
            return false;

        value = (sqVal != 0);
        return true;
    }

    bool Runtime::FunctionContext::getParamValue(const int paramIndex, float& value) const
    {
        if (paramIndex < 0 || paramIndex >= mNumParams)
            return false;

        return SQ_SUCCEEDED(sq_getfloat(mSqvm, paramIndex + 2, &value));
    }

    bool Runtime::FunctionContext::getParamValue(const int paramIndex, std::string& value) const
    {
        if (paramIndex < 0 || paramIndex >= mNumParams)
            return false;

        const SQChar* sqVal;
        if (!SQ_SUCCEEDED(sq_getstring(mSqvm, paramIndex + 2, &sqVal)))
            return false;

        value = hr::StringUtils::conv2UTF8(sqVal);
        return true;
    }

    Runtime::FunctionReturnContext::FunctionReturnContext(HSQUIRRELVM& sqvm)
      : FunctionContext(sqvm), mReturnValueSet(false)
    {}

    void Runtime::FunctionReturnContext::setReturnValue()
    {
        if (mReturnValueSet || !mErrorThrown.empty())
            return;

        sq_pushnull(mSqvm);
        mReturnValueSet = true;
    }

    void Runtime::FunctionReturnContext::setReturnValue(const int value)
    {
        if (mReturnValueSet || !mErrorThrown.empty())
            return;

        sq_pushinteger(mSqvm, value);
        mReturnValueSet = true;
    }

    void Runtime::FunctionReturnContext::setReturnValue(const bool value)
    {
        if (mReturnValueSet || !mErrorThrown.empty())
            return;

        sq_pushbool(mSqvm, value);
        mReturnValueSet = true;
    }

    void Runtime::FunctionReturnContext::setReturnValue(const float value)
    {
        if (mReturnValueSet || !mErrorThrown.empty())
            return;

        sq_pushfloat(mSqvm, value);
        mReturnValueSet = true;
    }

    void Runtime::FunctionReturnContext::setReturnValue(const char* const value)
    {
        if (mReturnValueSet || !mErrorThrown.empty())
            return;

        sq_pushstring(mSqvm, hr::StringUtils::conv2Native(value).c_str(), -1);
        mReturnValueSet = true;
    }

    void Runtime::FunctionReturnContext::setReturnValue(const std::string& value)
    {
        if (mReturnValueSet || !mErrorThrown.empty())
            return;

        auto valueWChar = hr::StringUtils::conv2Native(value);

        sq_pushstring(mSqvm, valueWChar.c_str(), valueWChar.size());
        mReturnValueSet = true;
    }

    void Runtime::FunctionReturnContext::setReturnValue(const ClassProxy& value)
    {
        if (mReturnValueSet || !mErrorThrown.empty())
            return;

        sq_pushobject(mSqvm, value.mVMInstance);
        mReturnValueSet = true;
    }

    void Runtime::FunctionReturnContext::setReturnValue(const std::vector<int>& values)
    {
        if (mReturnValueSet || !mErrorThrown.empty())
            return;

        sq_newarray(mSqvm, values.size());

        int curIndex = 0;
        for (const auto& value : values)
        {
            sq_pushinteger(mSqvm, curIndex);
            sq_pushinteger(mSqvm, value);
            sq_set(mSqvm, -3);

            curIndex++;
        }

        mReturnValueSet = true;
    }

    void Runtime::FunctionReturnContext::setReturnValue(const std::vector<bool>& values)
    {
        if (mReturnValueSet || !mErrorThrown.empty())
            return;

        sq_newarray(mSqvm, values.size());

        int curIndex = 0;
        for (const auto& value : values)
        {
            sq_pushinteger(mSqvm, curIndex);
            sq_pushbool(mSqvm, value);
            sq_set(mSqvm, -3);

            curIndex++;
        }

        mReturnValueSet = true;
    }

    void Runtime::FunctionReturnContext::setReturnValue(const std::vector<float>& values)
    {
        if (mReturnValueSet || !mErrorThrown.empty())
            return;

        sq_newarray(mSqvm, values.size());

        int curIndex = 0;
        for (const auto& value : values)
        {
            sq_pushinteger(mSqvm, curIndex);
            sq_pushfloat(mSqvm, value);
            sq_set(mSqvm, -3);

            curIndex++;
        }

        mReturnValueSet = true;
    }

    void Runtime::FunctionReturnContext::setReturnValue(const std::vector<std::string>& values)
    {
        if (mReturnValueSet || !mErrorThrown.empty())
            return;

        sq_newarray(mSqvm, values.size());

        int curIndex = 0;
        for (const auto& value : values)
        {
            auto valueWChar = hr::StringUtils::conv2Native(value);

            sq_pushinteger(mSqvm, curIndex);
            sq_pushstring(mSqvm, valueWChar.c_str(), valueWChar.size());
            sq_set(mSqvm, -3);

            curIndex++;
        }

        mReturnValueSet = true;
    }

    void Runtime::ClassProxy::refIncrement()
    {
        sq_addref(mVM, &mVMInstance);
    }

    bool Runtime::ClassProxy::refDecrement()
    {
        return (sq_release(mVM, &mVMInstance) == SQTrue);
    }

    void Runtime::ClassProxy::bindTable(const std::string& varName)
    {
        if (varName.empty())
            return;

        auto varNameWChar = hr::StringUtils::conv2Native(varName);

        sq_pushobject(mVM, mVMInstance);
        sq_pushstring(mVM, varNameWChar.c_str(), varNameWChar.size());
        sq_newtable(mVM);
        sq_set(mVM, -3);
        sq_pop(mVM, 1);
    }

    void Runtime::ClassProxy::bindValue(const std::string& varName, const int& value)
    {
        if (varName.empty())
            return;

        auto varNameWChar = hr::StringUtils::conv2Native(varName);

        sq_pushobject(mVM, mVMInstance);
        sq_pushstring(mVM, varNameWChar.c_str(), varNameWChar.size());
        sq_pushinteger(mVM, value);
        sq_set(mVM, -3);
        sq_pop(mVM, 1);
    }

    void Runtime::ClassProxy::bindValue(const std::string& varName, const bool& value)
    {
        if (varName.empty())
            return;

        auto varNameWChar = hr::StringUtils::conv2Native(varName);

        sq_pushobject(mVM, mVMInstance);
        sq_pushstring(mVM, varNameWChar.c_str(), varNameWChar.size());
        sq_pushbool(mVM, value);
        sq_set(mVM, -3);
        sq_pop(mVM, 1);
    }

    void Runtime::ClassProxy::bindValue(const std::string& varName, const float& value)
    {
        if (varName.empty())
            return;

        auto varNameWChar = hr::StringUtils::conv2Native(varName);

        sq_pushobject(mVM, mVMInstance);
        sq_pushstring(mVM, varNameWChar.c_str(), varNameWChar.size());
        sq_pushfloat(mVM, value);
        sq_set(mVM, -3);
        sq_pop(mVM, 1);
    }

    void Runtime::ClassProxy::bindValue(const std::string& varName, const char* const value)
    {
        if (varName.empty())
            return;

        auto varNameWChar = hr::StringUtils::conv2Native(varName);
        auto valueWChar = hr::StringUtils::conv2Native(value);

        sq_pushobject(mVM, mVMInstance);
        sq_pushstring(mVM, varNameWChar.c_str(), varNameWChar.size());
        sq_pushstring(mVM, valueWChar.c_str(), valueWChar.size());
        sq_set(mVM, -3);
        sq_pop(mVM, 1);
    }

    void Runtime::ClassProxy::bindValue(const std::string& varName, const std::string& value)
    {
        if (varName.empty())
            return;

        auto varNameWChar = hr::StringUtils::conv2Native(varName);
        auto valueWChar = hr::StringUtils::conv2Native(value);

        sq_pushobject(mVM, mVMInstance);
        sq_pushstring(mVM, varNameWChar.c_str(), varNameWChar.size());
        sq_pushstring(mVM, valueWChar.c_str(), valueWChar.size());
        sq_set(mVM, -3);
        sq_pop(mVM, 1);
    }

    void Runtime::parsePath(const char* const fullPath, std::function<bool(const std::string&, bool)> cbTokenFound)
    {
        if (!cbTokenFound)
            return;

        auto tokenStart = fullPath;
        auto tokenEnd = fullPath;
        while (*tokenEnd != '\0')
        {
            while (*tokenEnd != '\0' && *tokenEnd != '.')
                tokenEnd++;

            std::string token(tokenStart, tokenEnd - tokenStart);
            auto isLastToken = (*tokenEnd == '\0');

            auto continueParse = cbTokenFound(token, isLastToken);
            if (!continueParse)
                break;

            if (isLastToken)
                break;

            tokenEnd++;
            tokenStart = tokenEnd;
        }
    }

    bool Runtime::pushFullPathToStack(HSQUIRRELVM vm, const char* const fullPath, bool checkPathExists, std::function<void(const std::wstring&)> cbOnPathEnd)
    {
        if (!fullPath || *fullPath == '\0' || !cbOnPathEnd)
            return false;

        sq_pushroottable(vm);
        sq_pushstring(vm, _SC("engine"), -1);
        sq_get(vm, -2);

        int objsOnStack = 2;
        bool pathNotFound = false;

        Runtime::parsePath(fullPath,
                           [&](const std::string& token, bool isLastToken) -> bool
                           {
                               auto tokenWChar = hr::StringUtils::conv2Native(token);

                               if (isLastToken)
                               {
                                   cbOnPathEnd(tokenWChar);
                                   return true;
                               }

                               sq_pushstring(vm, tokenWChar.c_str(), tokenWChar.size());
                               if (SQ_FAILED(sq_get(vm, -2)))
                               {
                                   if (!checkPathExists)
                                   {
                                       pathNotFound = true;
                                       return false;
                                   }

                                   sq_pushstring(vm, tokenWChar.c_str(), tokenWChar.size());
                                   sq_newtable(vm);
                                   sq_newslot(vm, -3, SQFalse);

                                   sq_pushstring(vm, tokenWChar.c_str(), tokenWChar.size());
                                   sq_get(vm, -2);
                               }

                               objsOnStack++;
                               return true;
                           });

        if (pathNotFound)
            return false;

        sq_pop(vm, objsOnStack);
        return true;
    }

    void Runtime::squirrelStackTrace(HSQUIRRELVM vm)
    {
        SQInteger stack_depth;
        SQStackInfos stack_info;

        auto runtime = reinterpret_cast<Runtime*>(sq_getforeignptr(vm));
        runtime->mLogger.error("Stack dump:");

        stack_depth = 1;
        while (SQ_SUCCEEDED(sq_stackinfos(vm, stack_depth, &stack_info)))
        {
            const SQChar* func_name = (stack_info.funcname) ? stack_info.funcname : _SC("unknown_function");
            const SQChar* source_file = (stack_info.source) ? stack_info.source : _SC("unknown_source_file");

            runtime->mLogger.error("[{0}]: function [{1}()] {2}s line [{3}]", stack_depth, hr::StringUtils::conv2UTF8(func_name), hr::StringUtils::conv2UTF8(source_file),
                                   stack_info.line);

            stack_depth++;
        }
    }

    SQInteger Runtime::squirrelRuntimeError(HSQUIRRELVM vm)
    {
        if (sq_gettop(vm) >= 1)
        {
            const SQChar* error_message = NULL;
            if (SQ_SUCCEEDED(sq_getstring(vm, 2, &error_message)))
            {
                auto runtime = reinterpret_cast<Runtime*>(sq_getforeignptr(vm));
                runtime->mLogger.error(hr::StringUtils::conv2UTF8(error_message));
            }

            squirrelStackTrace(vm);
        }

        return 0;
    }

    void Runtime::squirrelCompileError(HSQUIRRELVM vm, const SQChar* description, const SQChar* file, SQInteger line, SQInteger column)
    {
        auto runtime = reinterpret_cast<Runtime*>(sq_getforeignptr(vm));
        runtime->mLogger.error("vm: '{0}' (Ln:{1} Col:{2}) : {3}.", hr::StringUtils::conv2UTF8(file), line, column, hr::StringUtils::conv2UTF8(description));
    }

    void Runtime::squirrelErrorFunction(HSQUIRRELVM vm, const SQChar* format, ...)
    {
        wchar_t buffer[1024 * 2];

        {
            va_list args;
            va_start(args, format);
            vswprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
        }

        auto runtime = reinterpret_cast<Runtime*>(sq_getforeignptr(vm));
        runtime->mLogger.error(hr::StringUtils::conv2UTF8(buffer));
    }

    void Runtime::squirrelPrintFunction(HSQUIRRELVM vm, const SQChar* format, ...)
    {
        wchar_t buffer[1024 * 2];

        {
            va_list args;
            va_start(args, format);
            vswprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
        }

        auto runtime = reinterpret_cast<Runtime*>(sq_getforeignptr(vm));
        runtime->mLogger.info(hr::StringUtils::conv2UTF8(buffer));
    }

    SQInteger Runtime::vmRegisteredFunc(HSQUIRRELVM sqvm)
    {
        auto runtime = reinterpret_cast<Runtime*>(sq_getforeignptr(sqvm));

        const SQChar* funcName;
        if (!SQ_SUCCEEDED(sq_getstring(sqvm, -1, &funcName)))
            return sq_throwerror(sqvm, _SC("Internal error - func name not found"));

        auto utf8FuncName = hr::StringUtils::conv2UTF8(funcName);

        auto funcTarget = runtime->mFuncMap[utf8FuncName];
        if (!funcTarget)
            return sq_throwerror(sqvm, _SC("Internal error - func id invalid"));

        FunctionReturnContext ctx(sqvm);

        funcTarget(utf8FuncName, ctx);
        if (ctx.mErrorThrown.empty())
            return (ctx.mReturnValueSet ? 1 : 0);

        if (ctx.mReturnValueSet)
            sq_pop(sqvm, 1);

        return sq_throwerror(sqvm, hr::StringUtils::conv2Native(ctx.mErrorThrown).c_str());
    }

    void Runtime::vmPrintLastError(HSQUIRRELVM sqvm)
    {
        auto runtime = reinterpret_cast<Runtime*>(sq_getforeignptr(sqvm));

        const SQChar* error;
        sq_getlasterror(sqvm);
        if (SQ_SUCCEEDED(sq_getstring(sqvm, -1, &error)))
            runtime->mLogger.error(hr::StringUtils::conv2UTF8(error));
    }

    void Runtime::vmPushStackValue(HSQUIRRELVM sqvm, const int value)
    {
        sq_pushinteger(sqvm, value);
    }

    void Runtime::vmPushStackValue(HSQUIRRELVM sqvm, const bool value)
    {
        sq_pushbool(sqvm, value);
    }

    void Runtime::vmPushStackValue(HSQUIRRELVM sqvm, const float value)
    {
        sq_pushfloat(sqvm, value);
    }

    void Runtime::vmPushStackValue(HSQUIRRELVM sqvm, const char* const value)
    {
        auto valueWChar = hr::StringUtils::conv2Native(value);
        sq_pushstring(sqvm, valueWChar.c_str(), valueWChar.size());
    }

    void Runtime::vmPushStackValue(HSQUIRRELVM sqvm, const std::string& value)
    {
        auto valueWChar = hr::StringUtils::conv2Native(value);
        sq_pushstring(sqvm, valueWChar.c_str(), valueWChar.size());
    }

    Runtime::Runtime(Logger::Context& logger)
      : mLogger(logger)
    {
        mVM = sq_open(1024);
        sq_setforeignptr(mVM, this);
        sq_setprintfunc(mVM, squirrelPrintFunction, squirrelErrorFunction);
        sq_setcompilererrorhandler(mVM, squirrelCompileError);

        sq_newclosure(mVM, squirrelRuntimeError, 0);
        sq_seterrorhandler(mVM);

        sq_pushroottable(mVM); // everything new goes here

        //::utils
        sq_pushstring(mVM, _SC("utils"), -1);
        sq_newtable(mVM);
        if (SQ_FAILED(sqstd_register_bloblib(mVM)) || SQ_FAILED(sqstd_register_mathlib(mVM)) || SQ_FAILED(sqstd_register_stringlib(mVM)))
            return;
        sq_newslot(mVM, -3, SQFalse);

        //::engine
        sq_pushstring(mVM, _SC("engine"), -1);
        sq_newtable(mVM);
        sq_newslot(mVM, -3, SQFalse);

        sq_pop(mVM, 1); // pop the root table
    }

    Runtime::~Runtime()
    {
        sq_close(mVM);
        mFuncMap.clear();
    }

    bool Runtime::registerFunc(const char* const funcName, RegisteredFuncType funcCallback)
    {
        if (!funcName || funcName[0] == '\0' || !funcCallback)
            return false;

        return Runtime::pushFullPathToStack(mVM, funcName, true,
                                            [&](const std::wstring& lastToken)
                                            {
                                                auto fullFuncName = std::string(funcName);
                                                auto fullFuncNameWChar = hr::StringUtils::conv2Native(fullFuncName);

                                                sq_pushstring(mVM, lastToken.c_str(), lastToken.size());
                                                sq_pushstring(mVM, fullFuncNameWChar.c_str(), fullFuncNameWChar.size()); // free var
                                                sq_newclosure(mVM, Runtime::vmRegisteredFunc, 1);
                                                sq_setnativeclosurename(mVM, -1, lastToken.c_str()); // debug purposes
                                                sq_newslot(mVM, -3, SQFalse);

                                                mFuncMap[fullFuncName] = funcCallback;
                                            });
    }

    void Runtime::unregisterFunc(const char* const funcName)
    {
        abort();
    }

    bool Runtime::runScript(const char* const script)
    {
        if (!script || *script == '\0')
            return false;

        sq_pushroottable(mVM);

        {
            auto scriptWChar = hr::StringUtils::conv2Native(script);

            if (SQ_FAILED(sq_compilebuffer(mVM, scriptWChar.c_str(), scriptWChar.size(), _SC("main runtime"), SQTrue)))
            {
                vmPrintLastError(mVM);
                return false;
            }
        }

        sq_pushroottable(mVM);
        if (SQ_FAILED(sq_call(mVM, 1, SQFalse, SQTrue)))
        {
            vmPrintLastError(mVM);
            return false;
        }

        sq_pop(mVM, 1);
        return true;
    }

    bool Runtime::runScriptFile(const char* const filePath)
    {
        sq_pushroottable(mVM);

        {
            auto script = hr::streams::FileStream::readEntireFileAsString(filePath);
            if (!script)
                return false;

            auto scriptWChar = hr::StringUtils::conv2Native(script.value());

            if (SQ_FAILED(sq_compilebuffer(mVM, scriptWChar.c_str(), scriptWChar.size(), _SC("main runtime"), SQTrue)))
            {
                vmPrintLastError(mVM);
                return false;
            }
        }

        sq_pushroottable(mVM);
        if (SQ_FAILED(sq_call(mVM, 1, SQFalse, SQTrue)))
        {
            vmPrintLastError(mVM);
            return false;
        }

        sq_pop(mVM, 1);
        return true;
    }
}

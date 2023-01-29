#pragma once

#include "runtime.hpp"
#include "logger.hpp"

#include "common/tasks.hpp"
#include "common/timer.hpp"
#include "common/types.hpp"
#include "common/avl-tree.hpp"
#include "common/fileSystem.hpp"

#include "platform/window.hpp"

#include <array>
#include <memory>
#include <string>

namespace hr { namespace engine
{
	class Engine final
	{
	public:
		enum class State { Created, Initializing, Running, Stopping, Stopped, StoppedError };
		enum class ExitAction { Nothing, Restart };
		enum class StatSampleType { Fps, NumTris };

		class IVariable
		{
		public:
			enum class VariableType { Bool, Integer, Double, String };

		protected:
			VariableType mType;
			std::string mDesc;

		public:
			IVariable(const VariableType varType)
				: mType(varType)
			{}

			IVariable(const VariableType varType, const std::string &desc)
				: mType(varType), mDesc(desc)
			{}

			IVariable(const IVariable&) = delete;
			IVariable& operator=(const IVariable&) = delete;

			bool isType(VariableType varType) const
			{
				return (mType == varType);
			}

			VariableType type() const
			{
				return mType;
			}

			const std::string& desc() const
			{
				return mDesc;
			}
		};

		class VariableBool final : public IVariable
		{
			bool mValue = false;

		public:
			VariableBool(const bool defaultValue)
				: IVariable(IVariable::VariableType::Bool), mValue(defaultValue)
			{}

			VariableBool(const bool defaultValue, const std::string& desc)
				: IVariable(IVariable::VariableType::Bool, desc), mValue(defaultValue)
			{}

			VariableBool(const VariableBool&) = delete;
			VariableBool& operator=(const VariableBool&) = delete;

			bool value() const
			{
				return mValue;
			}

			bool value(const bool newValue)
			{
				mValue = newValue;
				return mValue;
			}
		};

		class VariableInt final : public IVariable
		{
			int64_t mValue = 0;
			int64_t mClampMin = 0, mClampMax = 0;
			bool mClampValue = false;

		public:
			VariableInt(const int64_t defaultValue)
				: IVariable(IVariable::VariableType::Integer), mValue(defaultValue)
			{}

			VariableInt(const int64_t defaultValue, const std::string& desc)
				: IVariable(IVariable::VariableType::Integer, desc), mValue(defaultValue)
			{}

			VariableInt(const int64_t defaultValue, const int64_t clampMinValue, const int64_t clampMaxValue)
				: IVariable(IVariable::VariableType::Integer), mValue(defaultValue), mClampMin(clampMinValue), mClampMax(clampMaxValue), mClampValue(true)
			{}

			VariableInt(const int64_t defaultValue, const int64_t clampMinValue, const int64_t clampMaxValue, const std::string& desc)
				: IVariable(IVariable::VariableType::Integer, desc), mValue(defaultValue), mClampMin(clampMinValue), mClampMax(clampMaxValue), mClampValue(true)
			{}

			VariableInt(const VariableInt&) = delete;
			VariableInt& operator=(const VariableInt&) = delete;

			int64_t value() const
			{
				return mValue;
			}

			int64_t value(const int64_t newValue)
			{
				mValue = newValue;
				if (mClampValue)
					mValue = mValue < mClampMin ? mClampMin : (mValue > mClampMax ? mClampMax : mValue);

				return mValue;
			}
		};

		class VariableDouble final : public IVariable
		{
			double mValue = 0.0;
			double mClampMin = 0.0, mClampMax = 0.0;
			bool mClampValue = false;

		public:
			VariableDouble(const double defaultValue)
				: IVariable(IVariable::VariableType::Double), mValue(defaultValue)
			{}

			VariableDouble(const double defaultValue, const std::string& desc)
				: IVariable(IVariable::VariableType::Double, desc), mValue(defaultValue)
			{}

			VariableDouble(const double defaultValue, const double clampMinValue, const double clampMaxValue)
				: IVariable(IVariable::VariableType::Double), mValue(defaultValue), mClampMin(clampMinValue), mClampMax(clampMaxValue), mClampValue(true)
			{}

			VariableDouble(const double defaultValue, const double clampMinValue, const double clampMaxValue, const std::string& desc)
				: IVariable(IVariable::VariableType::Double, desc), mValue(defaultValue), mClampMin(clampMinValue), mClampMax(clampMaxValue), mClampValue(true)
			{}

			VariableDouble(const VariableDouble&) = delete;
			VariableDouble& operator=(const VariableDouble&) = delete;

			double value() const
			{
				return mValue;
			}

			double value(const double newValue)
			{
				mValue = newValue;
				if (mClampValue)
					mValue = mValue < mClampMin ? mClampMin : (mValue > mClampMax ? mClampMax : mValue);

				return mValue;
			}
		};

		class VariableString final : public IVariable
		{
			std::string mValue;

		public:
			VariableString(const std::string& defaultValue)
				: IVariable(IVariable::VariableType::String), mValue(defaultValue)
			{}

			VariableString(const std::string& defaultValue, const std::string& desc)
				: IVariable(IVariable::VariableType::String, desc), mValue(defaultValue)
			{}

			VariableString(const VariableString&) = delete;
			VariableString& operator=(const VariableString&) = delete;

			const std::string& value() const
			{
				return mValue;
			}

			const std::string& value(const std::string& newValue)
			{
				mValue = newValue;
				return mValue;
			}
		};

	private:
		template<std::size_t N>
		class StatSeries
		{
			double mSampleAccum;
			size_t mNumSamples;

			size_t mTotalHistory;
			size_t mIndexHistory;
			std::array<float, N> mHistory;

			std::string mUnits;

		public:
			StatSeries(const std::string &units)
				: mSampleAccum(0.0), mNumSamples(0), mTotalHistory(0), mIndexHistory(0), mUnits(units)
			{ }

			StatSeries(const StatSeries&) = delete;
			StatSeries& operator=(const StatSeries&) = delete;

			void accumSample(const float value)
			{
				mSampleAccum += value;
				mNumSamples++;
			}

			void moveNextSample()
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

			float getLastSample() const
			{
				if (mTotalHistory <= 0)
					return 0.0f;

				return ((mIndexHistory == 0) ? mHistory[N - 1] : mHistory[mIndexHistory - 1]);
			}

			const std::string& getUnits() const
			{
				return mUnits;
			}
		};

	private:
		mutable std::mutex mSyncLock;

		bool mDevMode{false};
		std::string mErrorDesc;
		std::atomic<State> mCurState{State::Created};
		hr::Timer mMainTimer;
		int mExitCode{0};
		std::atomic<ExitAction> mExitAction{ExitAction::Nothing};
		AVLTree<char, std::shared_ptr<IVariable>> mVars;
		std::unordered_map<StatSampleType, std::unique_ptr<StatSeries<3000>>> mStats;

		Scheduler mAsyncScheduler;
		Dispatcher mAsyncDispatcher;
		std::vector<std::jthread> mAsyncThreads;

		std::shared_ptr<Logger> mLogger;
		std::shared_ptr<Logger::Context> mLoggerRenderCtx;
		std::shared_ptr<Logger::Context> mLoggerRuntimeCtx;

		std::shared_ptr<platform::Window> mWindow;
		std::shared_ptr<Runtime> mRuntime;
		std::shared_ptr<io::FileSystem> mFileSystem;

		struct
		{
			bool dbgRayTrace{false};
		} mCmdLineOptions;

	private:
		void exit(ExitAction exitAction, const char * const errorDesc = nullptr);

		void initParseCmdLine(std::string_view cmdLine);
		void initFileSystem();

		void renderLoop();

		void runtimeFuncVarRegister(const std::string &funcName, Runtime::FunctionReturnContext &ctx);
		void runtimeFuncVarGet(const std::string &funcName, Runtime::FunctionReturnContext &ctx);
		void runtimeFuncVarSet(const std::string &funcName, Runtime::FunctionReturnContext &ctx);
		void runtimeFuncVarList(const std::string &funcName, Runtime::FunctionReturnContext &ctx);
		void runtimeFuncRuntime(const std::string &funcName, Runtime::FunctionReturnContext &ctx);

		void logSysInfo();

	public:
		Engine(std::string_view cmdLine, bool devMode = false);
		~Engine();

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		// vars
		template<typename T>
		T var(const char* const name) const
		{
			//static_assert(false, "variable type not supported");
		}

		template<typename T>
		void var(const char* const name, const T& value)
		{
			//static_assert(false, "variable type not supported");
		}

		bool mainLoop();

		Dispatcher& asyncDispatcher() noexcept
		{
			return mAsyncDispatcher;
		}

		int getExitCode() const;
		ExitAction getExitAction() const;

		std::string getErrorDesc() const;
	};
} }

template<> bool hr::engine::Engine::var<bool>(const char* const name) const;
template<> int32_t hr::engine::Engine::var<int32_t>(const char* const name) const;
template<> int64_t hr::engine::Engine::var<int64_t>(const char* const name) const;
template<> float hr::engine::Engine::var<float>(const char* const name) const;
template<> double hr::engine::Engine::var<double>(const char* const name) const;
template<> std::string hr::engine::Engine::var<std::string>(const char* const name) const;

template<> void hr::engine::Engine::var<bool>(const char* const name, const bool& value);
template<> void hr::engine::Engine::var<int32_t>(const char* const name, const int32_t& value);
template<> void hr::engine::Engine::var<int64_t>(const char* const name, const int64_t& value);
template<> void hr::engine::Engine::var<float>(const char* const name, const float& value);
template<> void hr::engine::Engine::var<double>(const char* const name, const double& value);
template<> void hr::engine::Engine::var<std::string>(const char* const name, const std::string& value);

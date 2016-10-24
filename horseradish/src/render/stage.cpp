#include "render/stage.hpp"
#include "render/scene.hpp"

#include "common/Timer.hpp"
#include "common/opengl/tools/viewport.hpp"

#include <codecvt>
#include <locale>
#include <memory>

namespace HorseRadish { namespace Render {

	class Stage::SceneRuntimeProxy : public Engine::Runtime::ClassProxy
	{
	private:
		bool mActive = false;
		double mStepDelta = 0.0, mStepTotal = 0.0;
		Timer mTimerDelta, mTimerTotal;
		std::shared_ptr<Scene> mScene;

	private:
		static void runtimeFuncCb(SceneRuntimeProxy &target, const std::string &funcName, Engine::Runtime::FunctionReturnContext &ctx)
		{
			if (funcName == "sendMessage")
			{
				if (ctx.getNumParams() != 1 && ctx.getNumParams() != 2)
				{
					ctx.throwError("Invalid number of arguments");
					return;
				}

				std::string msg, payload;
				ctx.getParamValue(0, msg);
				if (ctx.getNumParams() == 2)
					ctx.getParamValue(0, payload);

				target.mScene->processMessage(msg, payload);
				return;
			}

			ctx.throwError("Unknown method");
		}

	public:
		static void runtimeRegisterData(Engine::Runtime::NativeClass<SceneRuntimeProxy>& nativeClass)
		{
			nativeClass.registerClassMethod("sendMessage", &SceneRuntimeProxy::runtimeFuncCb);
			nativeClass.registerClassVar("events");
		}

	public:
		SceneRuntimeProxy(HSQUIRRELVM vm, HSQOBJECT vmInstance, Stage &stage, const std::string& sceneName, const std::string& sceneFilePath)
			: Engine::Runtime::ClassProxy(vm, vmInstance)
		{
			bindTable("events");

			mScene = std::make_shared<Scene>(stage.mRuntime, stage.mLogger, stage.mFileSystem, stage.mGlCtx, sceneName, sceneFilePath, stage.mRenderData.texColor.width(), stage.mRenderData.texColor.height());
		}

		SceneRuntimeProxy(HSQUIRRELVM vm, HSQOBJECT vmInstance, Engine::Runtime::FunctionContext &ctx)
			: Engine::Runtime::ClassProxy(vm, vmInstance)
		{
			ctx.throwError("Scenes should only be create through the stage API");
		}

		~SceneRuntimeProxy()
		{ }

		bool isActive() const
		{
			return mActive;
		}

		Scene& getScene() const
		{
			return *mScene;
		}

		double getStepDeltaMS() const
		{
			return mStepDelta;
		}

		double getStepTotalMS() const
		{
			return mStepTotal;
		}

		void setActiveState(bool isActive)
		{
			if (mActive == isActive)
				return;

			mActive = isActive;
			if (mActive)
			{
				mTimerDelta.reStart();
				mTimerTotal.reStart();
				mStepDelta = mStepTotal = 0.0;
			}
		}

		void processStep()
		{
			mStepDelta = mTimerDelta.getTimeMS(true);
			mStepTotal = mTimerTotal.getTimeMS();
		}

		void rtInvokeDraw()
		{
			this->invokeVoidMethod("events.onDraw");
		}

		void rtInvokeStep()
		{
			this->invokeVoidMethod("events.onStep", static_cast<float>(mStepDelta), static_cast<float>(mStepTotal));
		}

		void rtInvokeMessage(const std::string &msg, const std::string &payload)
		{
			this->invokeVoidMethod("events.onMessage", msg, payload);
		}

		void rtInvokeMessage(const Window::Message& msg)
		{
			if (msg.isType(Window::Message::MessageType::CharacterKey))
				this->invokeVoidMethod("events.onKeyPress", msg.getParam());
		}
	};

	void Stage::runtimeFuncSceneGet(const std::string &funcName, Engine::Runtime::FunctionReturnContext &ctx)
	{
		if ((ctx.getNumParams() != 1) || (ctx.getParamType(0) != Engine::Runtime::FunctionContext::ParamType::String))
		{
			ctx.throwError("Invalid number of arguments");
			return;
		}

		std::string sceneName;
		if (!ctx.getParamValue(0, sceneName))
		{
			ctx.throwError("Invalid scene name");
			return;
		}

		auto it = mRuntimeScenes.find(sceneName);
		if (it == mRuntimeScenes.end())
			ctx.setReturnValue();
		else
			ctx.setReturnValue(*(it->second));
	}

	void Stage::runtimeFuncSceneCreate(const std::string &funcName, Engine::Runtime::FunctionReturnContext &ctx)
	{
		if (ctx.getNumParams() != 2)
		{
			ctx.throwError("Invalid number of arguments");
			return;
		}

		std::string sceneName, sceneFilePath;

		if (!ctx.getParamValue(0, sceneName))
		{
			ctx.throwError("Scene must have a valid name");
			return;
		}

		if (!ctx.getParamValue(1, sceneFilePath))
		{
			ctx.throwError("Scene must have a valid file path");
			return;
		}

		auto sceneInstanceProxy = ctx.createClassInstance<SceneRuntimeProxy>("CScene", *this, sceneName, sceneFilePath);
		if (!sceneInstanceProxy)
		{
			ctx.throwError("Unable to create CScene class instances");
			return;
		}

		mRuntimeScenes[sceneName] = sceneInstanceProxy;

		ctx.setReturnValue(*sceneInstanceProxy);
	}

	void Stage::runtimeFuncSceneDestroy(const std::string &funcName, Engine::Runtime::FunctionReturnContext &ctx)
	{
		if ((ctx.getNumParams() != 1) || (ctx.getParamType(0) != Engine::Runtime::FunctionContext::ParamType::ClassInstance))
		{
			ctx.throwError("Invalid number of arguments");
			return;
		}

		auto sceneProxy = ctx.getParamAsClassInstance<SceneRuntimeProxy>(0);
		if (!sceneProxy)
		{
			ctx.throwError("Invalid scene");
			return;
		}

		auto it = mRuntimeScenes.find(sceneProxy->getScene().getName());
		if (it == mRuntimeScenes.end())
			return;

		mRuntimeScenes.erase(it);
		auto emptyRef = sceneProxy->refDecrement();

		assert(emptyRef);
	}

	void Stage::runtimeFuncSceneAddRemove(const std::string &funcName, Engine::Runtime::FunctionReturnContext &ctx)
	{
		if ((ctx.getNumParams() != 1) || (ctx.getParamType(0) != Engine::Runtime::FunctionContext::ParamType::ClassInstance))
		{
			ctx.throwError("Invalid number of arguments");
			return;
		}

		auto sceneProxy = ctx.getParamAsClassInstance<SceneRuntimeProxy>(0);
		if (!sceneProxy)
		{
			ctx.throwError("Invalid scene");
			return;
		}

		sceneProxy->setActiveState((funcName == "stage.add"));
	}

	Stage::Stage(Engine::Runtime& runtime, Engine::Logger::Context& logger, HorseRadish::IO::FileSystem& fileSystem, HorseRadish::OpenGL::Objects::Context &glCtx, size_t renderWidth, size_t renderHeight)
		: mRuntime(runtime)
		, mLogger(logger)
		, mFileSystem(fileSystem)
		, mGlCtx(glCtx)
	{
		//render
		mRenderData.texColor.init(HorseRadish::OpenGL::Objects::Texture::Type::TexRectangle, HorseRadish::OpenGL::Objects::Texture::StorageType::RGBA_8, renderWidth, renderHeight);

		mRenderData.fbo.init();
		mRenderData.fbo.attachTColor(mRenderData.texColor, 0);
		mRenderData.fbo.isStatusComplete();

		mRenderData.sampler.init();
		mRenderData.sampler.setMinFilter(HorseRadish::OpenGL::Objects::Sampler::FilterType::Point);
		mRenderData.sampler.setMagFilter(HorseRadish::OpenGL::Objects::Sampler::FilterType::Point);
		mRenderData.sampler.setWrap(HorseRadish::OpenGL::Objects::Sampler::WrapType::ClampEdge);

		mRenderData.progVertex.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Vertex, fileSystem.readFileAsString("shaders/stage.vshader"));
		mRenderData.progFragment.init(HorseRadish::OpenGL::Objects::ShaderProgram::Type::Fragment, fileSystem.readFileAsString("shaders/stage.fshader"));

		auto matrixProj2D = HorseRadish::OpenGL::Tools::Viewport::genMatrix2DProj(renderWidth, renderHeight);
		HorseRadish::OpenGL::glProgramUniform1i(mRenderData.progFragment.getId(), mRenderData.progFragment.getUniformLocation("texSampler"), 0);
		HorseRadish::OpenGL::glProgramUniformMatrix4fv(mRenderData.progVertex.getId(), mRenderData.progVertex.getUniformLocation("transformationMatrix"), 1, false, matrixProj2D.data());

		mRenderData.progPipeline.init();
		mRenderData.progPipeline.setStage(mRenderData.progVertex);
		mRenderData.progPipeline.setStage(mRenderData.progFragment);

		//take care of the runtime
		mRuntime.registerFunc("stage.getScene", std::bind(&Stage::runtimeFuncSceneGet, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime.registerFunc("stage.create", std::bind(&Stage::runtimeFuncSceneCreate, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime.registerFunc("stage.destroy", std::bind(&Stage::runtimeFuncSceneDestroy, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime.registerFunc("stage.add", std::bind(&Stage::runtimeFuncSceneAddRemove, this, std::placeholders::_1, std::placeholders::_2));
		mRuntime.registerFunc("stage.remove", std::bind(&Stage::runtimeFuncSceneAddRemove, this, std::placeholders::_1, std::placeholders::_2));

		mRuntime.registerClass<SceneRuntimeProxy>("CScene");
	}

	Stage::~Stage()
	{
		std::memset(&mRenderData, 0, sizeof(RenderData));
	}

	void Stage::drawScenes()
	{
		for (auto& sceneIt : mRuntimeScenes)
		{
			if (sceneIt.second->isActive())
				mTempScenes.push_back(sceneIt.second);
		}

		mRenderData.texColor.clear(0.0f, 0.0f, 0.0f, 1.0f);
		mRenderData.fbo.bind();

		for (auto& scene : mTempScenes)
			mScenesDrawned |= scene->getScene().processDraw();

		HorseRadish::OpenGL::glBindFramebuffer(GL_FRAMEBUFFER, 0);

		for (auto& scene : mTempScenes)
			scene->rtInvokeDraw();

		mTempScenes.clear();
	}

	void Stage::drawComposite(const HorseRadish::OpenGL::Tools::Viewport& viewport)
	{
		if (!mScenesDrawned)
			return;

		mScenesDrawned = false;

		HorseRadish::OpenGL::glUseProgram(0);
		HorseRadish::OpenGL::glBindProgramPipeline(mRenderData.progPipeline.getId());

		mRenderData.sampler.bind(0);
		mRenderData.texColor.bind(0);

		mRenderData.imode.beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
			mRenderData.imode.setColorF(1.0f, 1.0f, 1.0f, 1.0f);
			mRenderData.imode.addQuadTexCoords(0.0f, 0.0f, static_cast<float>(viewport.width()), static_cast<float>(viewport.height()), false);
		mRenderData.imode.endDraw();
	}

	void Stage::processStep()
	{
		//the scene step (can invoke runtime)

		for (auto& sceneIt : mRuntimeScenes)
		{
			if (sceneIt.second->isActive())
			{
				sceneIt.second->processStep();
				mTempScenes.push_back(sceneIt.second);
			}
		}

		for (auto& scene : mTempScenes)
		{
			std::list<std::pair<std::string, std::string>> msgs;

			scene->getScene().processStep(scene->getStepDeltaMS(), scene->getStepTotalMS(), [&](const std::string &msg, const std::string &payload)
			{
				msgs.push_back(std::make_pair(msg, payload));
			});
		
			for (auto& msg : msgs)
				scene->rtInvokeMessage(msg.first, msg.second);
		}

		mTempScenes.clear();

		//the runtime step

		for (auto& sceneIt : mRuntimeScenes)
		{
			if (sceneIt.second->isActive())
				mTempScenes.push_back(sceneIt.second);
		}

		for (auto& scene : mTempScenes)
			scene->rtInvokeStep();

		mTempScenes.clear();
	}

	void Stage::processMessage(const Window::Message& msg)
	{
		for (auto& sceneIt : mRuntimeScenes)
		{
			if (sceneIt.second->isActive())
				mTempScenes.push_back(sceneIt.second);
		}

		for (auto& scene : mTempScenes)
			scene->rtInvokeMessage(msg);

		mTempScenes.clear();
	}
} }
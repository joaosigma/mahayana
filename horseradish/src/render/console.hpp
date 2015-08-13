#pragma once

#include "common/Timer.hpp"
#include "common/Primitives2D.hpp"
#include "common/opengl/objects.hpp"
#include "common/openGL/tools/viewport.hpp"
#include "render/renderer2D.hpp"

#include "../platform/window.hpp"

#include "engine/logger.hpp"

namespace HorseRadish { namespace Render {

class Console
{
	struct {
		int offset;
	} mLogView;

	struct {
		int offset;
		bool isVisivel;
		HorseRadish::Timer timer;
	} mCursor;

	struct {
		unsigned int historyOffset;
		unsigned int maxHistorySize;
		std::deque<std::string> history;
		std::vector<unsigned int> promptUnicode;
	} mPrompt;

	bool mMainVisible;
	std::deque<std::string> mAlerts;
	HorseRadish::Render::Renderer2D& mRenderer;
	const HorseRadish::Engine::Logger& mLogger;
	HorseRadish::Primitives2D::Rectangle<float> mViewRect, mTextRect;
	
	void updateCursorOffset(int offset);
	void processMsgPrompt(const Window::Message &msg);

	void drawContent(const HorseRadish::Matrix &transformMatrix) const;
	void drawBackground(const HorseRadish::Matrix &transformMatrix, float bkgAlpha) const;

public:
	Console(const HorseRadish::Engine::Logger& logger, HorseRadish::Render::Renderer2D& renderer, unsigned int maxPromptHistory);

	void draw(const HorseRadish::OpenGL::Tools::Viewport& viewport) const;

	bool isVisible() const;
	void setVisible(bool visible);

	void processStep();
	void processMsg(std::function<void(const char * const)> execPromptCmdCb, const Window::Message &msg);
};

} }


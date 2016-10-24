#pragma once

#include "common/timer.hpp"
#include "common/primitives2D.hpp"
#include "common/opengl/objects.hpp"
#include "common/openGL/tools/viewport.hpp"
#include "render/renderer2D.hpp"

#include "../platform/window.hpp"

#include "engine/logger.hpp"

namespace HorseRadish { namespace Render {

class ConsoleUI
{
	struct {
		int offset = 0;
	} mLogView;

	struct {
		int offset = 0;
		bool isVisivel = false;
		HorseRadish::Timer timer;
	} mCursor;

	struct {
		size_t historyOffset = 0;
		size_t maxHistorySize = 0;
		std::deque<std::string> history;
		std::vector<unsigned int> promptUnicode;
	} mPrompt;

	bool mMainVisible = false;
	std::deque<std::string> mAlerts;
	HorseRadish::Render::Renderer2D& mRenderer;
	const HorseRadish::Engine::Logger& mLogger;
	HorseRadish::Primitives2D::Rectangle<float> mViewRect, mTextRect;
	
	void updateCursorOffset(int offset);
	void processMsgPrompt(const Window::Message &msg);

	void drawContent(const HorseRadish::Matrix &transformMatrix) const;
	void drawBackground(const HorseRadish::Matrix &transformMatrix, float bkgAlpha) const;

public:
	ConsoleUI(const HorseRadish::Engine::Logger& logger, HorseRadish::Render::Renderer2D& renderer, size_t maxPromptHistory);

	void draw(const HorseRadish::OpenGL::Tools::Viewport& viewport) const;

	bool isVisible() const;
	void setVisible(bool visible);

	void processStep();
	void processMsg(const Window::Message &msg, std::function<void(const char * const)> execPromptCmdCb);
};

} }


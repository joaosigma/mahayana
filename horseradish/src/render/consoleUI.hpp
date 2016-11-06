#pragma once

#include "renderer2D.hpp"
#include "../common/timer.hpp"
#include "../common/primitives2D.hpp"
#include "../common/opengl/objects.hpp"
#include "../common/openGL/tools/viewport.hpp"
#include "../platform/window.hpp"
#include "../engine/logger.hpp"

namespace hr { namespace render
{
	class ConsoleUI
	{
		struct {
			int offset = 0;
		} mLogView;

		struct {
			int offset = 0;
			bool isVisivel = false;
			hr::Timer timer;
		} mCursor;

		struct {
			size_t historyOffset = 0;
			size_t maxHistorySize = 0;
			std::deque<std::string> history;
			std::vector<unsigned int> promptUnicode;
		} mPrompt;

		bool mMainVisible = false;
		std::deque<std::string> mAlerts;
		hr::render::Renderer2D& mRenderer;
		const hr::engine::Logger& mLogger;
		hr::Rectangle<float> mViewRect, mTextRect;
		
		void updateCursorOffset(int offset);
		void processMsgPrompt(const platform::Window::Message &msg);

		void drawContent(const hr::Matrix &transformMatrix) const;
		void drawBackground(const hr::Matrix &transformMatrix, float bkgAlpha) const;

	public:
		ConsoleUI(const hr::engine::Logger& logger, hr::render::Renderer2D& renderer, size_t maxPromptHistory);

		void draw(const hr::gl::tools::Viewport& viewport) const;

		bool isVisible() const;
		void setVisible(bool visible);

		void processStep();
		void processMsg(const platform::Window::Message &msg, std::function<void(const char * const)> execPromptCmdCb);
	};
} }

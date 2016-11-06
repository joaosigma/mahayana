#pragma once

#include "renderer2D.hpp"
#include "../common/timer.hpp"
#include "../common/primitives2D.hpp"
#include "../common/openGL/tools/viewport.hpp"
#include "../engine/profiler.hpp"

namespace hr { namespace render
{
	class ProfilerUI
	{
		bool mShowStats = false;
		bool mShowInfo = false;
		std::string mInfoStr;
		hr::Timer mTimer;
		hr::render::Renderer2D& mRenderer;
		const hr::engine::Profiler& mProfiler;
		
		void drawInfo(const hr::Rectangle<float>& viewRect, const hr::Matrix &transformMatrix) const;
		void drawStats(const hr::Rectangle<float>& viewRect, const hr::Matrix &transformMatrix) const;
		void drawStatsBackground(const hr::Rectangle<float>& viewRect, const hr::Matrix &transformMatrix, float bkgAlpha) const;

	public:
		ProfilerUI(const hr::engine::Profiler& profiler, hr::render::Renderer2D& renderer)
			: mRenderer(renderer), mProfiler(profiler)
		{ }

		void draw(const hr::gl::tools::Viewport& viewport) const;

		void processStats();

		bool isVisible() const;

		void setStatsState(bool enabled);
		void setInfoState(bool enabled);
	};
} }

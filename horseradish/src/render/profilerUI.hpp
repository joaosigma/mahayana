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
		
		void drawInfo(size_t textSize, const hr::Rectangle<float>& viewRect, const hr::Matrix4f& transformMatrix) const;
		void drawStats(size_t textSize, const hr::Rectangle<float>& viewRect, const hr::Matrix4f& transformMatrix) const;
		void drawStatsBackground(const hr::Rectangle<float>& viewRect, const hr::Matrix4f& transformMatrix, float bkgAlpha) const;

	public:
		ProfilerUI(const hr::engine::Profiler& profiler, hr::render::Renderer2D& renderer)
			: mRenderer(renderer), mProfiler(profiler)
		{ }

		void draw(size_t textSize, const hr::gl::tools::Viewport& viewport) const;

		void processStats();

		bool isVisible() const;

		void setStatsState(bool enabled);
		void setInfoState(bool enabled);
	};
} }

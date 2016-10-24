#pragma once

#include "common/timer.hpp"
#include "common/primitives2D.hpp"
#include "common/openGL/tools/viewport.hpp"
#include "render/renderer2D.hpp"

#include "engine/profiler.hpp"

namespace HorseRadish { namespace Render {

class ProfilerUI
{
	bool mShowStats, mShowInfo;
	std::string mInfoStr;
	HorseRadish::Timer mTimer;
	HorseRadish::Render::Renderer2D& mRenderer;
	const HorseRadish::Engine::Profiler& mProfiler;
	
	void drawInfo(const HorseRadish::Primitives2D::Rectangle<float>& viewRect, const HorseRadish::Matrix &transformMatrix) const;
	void drawStats(const HorseRadish::Primitives2D::Rectangle<float>& viewRect, const HorseRadish::Matrix &transformMatrix) const;
	void drawStatsBackground(const HorseRadish::Primitives2D::Rectangle<float>& viewRect, const HorseRadish::Matrix &transformMatrix, float bkgAlpha) const;

public:
	ProfilerUI(const HorseRadish::Engine::Profiler& profiler, HorseRadish::Render::Renderer2D& renderer);

	void draw(const HorseRadish::OpenGL::Tools::Viewport& viewport) const;

	void processStats();

	bool isVisible() const;

	void setStatsState(bool enabled);
	void setInfoState(bool enabled);
};

} }


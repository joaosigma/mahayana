#include "ProfilerUI.hpp"

#include "../common/color.hpp"

#include <libs/cppformat/format.h>

namespace HorseRadish { namespace Render {

std::array<HorseRadish::Engine::Profiler::StatId, 3> GraphStatIds = {
	HorseRadish::Engine::Profiler::StatId::FrameTotal,
	HorseRadish::Engine::Profiler::StatId::GPUSamples,
	HorseRadish::Engine::Profiler::StatId::GPUPrimitivesSubmitted };

static
HorseRadish::Color retrieveColor(HorseRadish::Engine::Profiler::StatId statId)
{
	const unsigned char *targetColor;

	switch (statId)
	{
	case HorseRadish::Engine::Profiler::StatId::FrameTotal:
		targetColor = HorseRadish::Color::KnownColors::CadetBlue;
		break;
	case HorseRadish::Engine::Profiler::StatId::FrameLogic:
		targetColor = HorseRadish::Color::KnownColors::BurlyWood;
		break;
	case HorseRadish::Engine::Profiler::StatId::FrameDraw:
		targetColor = HorseRadish::Color::KnownColors::Firebrick;
		break;
	case HorseRadish::Engine::Profiler::StatId::FrameGPU:
		targetColor = HorseRadish::Color::KnownColors::Coral;
		break;
	case HorseRadish::Engine::Profiler::StatId::GPUTimeElapsed:
		targetColor = HorseRadish::Color::KnownColors::DarkSalmon;
		break;
	case HorseRadish::Engine::Profiler::StatId::GPUSamples:
		targetColor = HorseRadish::Color::KnownColors::OliveDrab;
		break;
	case HorseRadish::Engine::Profiler::StatId::GPUVerticesSubmitted:
		targetColor = HorseRadish::Color::KnownColors::Wheat;
		break;
	case HorseRadish::Engine::Profiler::StatId::GPUPrimitivesSubmitted:
		targetColor = HorseRadish::Color::KnownColors::Sienna;
		break;
	case HorseRadish::Engine::Profiler::StatId::GPUVertexShaderInvocations:
		targetColor = HorseRadish::Color::KnownColors::PeachPuff;
		break;
	case HorseRadish::Engine::Profiler::StatId::GPUFragmentShaderInvocations:
		targetColor = HorseRadish::Color::KnownColors::BlanchedAlmond;
		break;
	case HorseRadish::Engine::Profiler::StatId::GPUClipInputPrimitives:
		targetColor = HorseRadish::Color::KnownColors::Cornsilk;
		break;
	case HorseRadish::Engine::Profiler::StatId::GPUClipOutputPrimitives:
		targetColor = HorseRadish::Color::KnownColors::MediumAquamarine;
		break;
	case HorseRadish::Engine::Profiler::StatId::MemTotal:
		targetColor = HorseRadish::Color::KnownColors::Lavender;
		break;
	case HorseRadish::Engine::Profiler::StatId::MemScripts:
		targetColor = HorseRadish::Color::KnownColors::Tan;
		break;
	case HorseRadish::Engine::Profiler::StatId::MemObjects:
		targetColor = HorseRadish::Color::KnownColors::Plum;
		break;
	case HorseRadish::Engine::Profiler::StatId::MemAnimations:
		targetColor = HorseRadish::Color::KnownColors::DodgerBlue;
		break;
	default:
		targetColor = HorseRadish::Color::KnownColors::DarkMagenta;
		break;
	}

	HorseRadish::Color color;
	color.set(targetColor);

	return color;
}

void ProfilerUI::drawInfo(const HorseRadish::Primitives2D::Rectangle<float>& viewRect, const HorseRadish::Matrix &transformMatrix) const
{
	if (mInfoStr.empty())
		return;

	auto& glImmediateMode = mRenderer.mGlImmediateMode;
	auto& guiFont = *mRenderer.mGui.font;

	float posX = viewRect.width - guiFont.getTextWidth(mInfoStr) - 10.0f;
	float posY = viewRect.height - guiFont.getMaxHeight() - 15.0f;

	HorseRadish::OpenGL::glUseProgram(0);
	HorseRadish::OpenGL::glBindProgramPipeline(mRenderer.mShaders.drawNoTex.progPipeline.getId());
	HorseRadish::OpenGL::glProgramUniformMatrix4fv(mRenderer.mShaders.drawNoTex.progVertex.getId(), mRenderer.mShaders.drawNoTex.progVertex.getUniformLocation("transformationMatrix"), 1, false, transformMatrix.data());

	glImmediateMode.beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::LineStrip);
		glImmediateMode.setColor(128, 128, 128);
		glImmediateMode.addPosition(viewRect.width, posY + guiFont.getMaxHeight() + 5.0f);
		glImmediateMode.addPosition(posX - 5.0f, posY + guiFont.getMaxHeight() + 5.0f);
		glImmediateMode.addPosition(posX - 5.0f, posY - 5.0f);
		glImmediateMode.addPosition(viewRect.width, posY - 5.0f);
	glImmediateMode.endDraw();

	guiFont.paintBegin(transformMatrix.data());
		guiFont.setColor(1.0f, 1.0f, 1.0f);
		guiFont.write(posX, posY, mInfoStr);
	guiFont.paintEnd();
}

void ProfilerUI::drawStats(const HorseRadish::Primitives2D::Rectangle<float>& viewRect, const HorseRadish::Matrix &transformMatrix) const
{
	if (GraphStatIds.empty())
		return;

	struct SampleLastValue
	{
		float posY;
		double value;
		HorseRadish::Engine::Profiler::StatId statId;
	};

	std::array<SampleLastValue, GraphStatIds.size()> samplePos;
	std::array<double, HorseRadish::Engine::Profiler::QuantizationSamples> sampleValues;

	auto& glImmediateMode = mRenderer.mGlImmediateMode;
	auto& guiFont = *mRenderer.mGui.font;

	auto textPreviewHeight = guiFont.getMaxHeight();
	auto textPreviewWidth = guiFont.getTextWidth("XXX.XX X") + 5.0f;

	HorseRadish::Primitives2D::Rectangle<float> graphRect(viewRect.x + 10.0f, viewRect.y + 10.0f, viewRect.width - 20.0f - textPreviewWidth, viewRect.height - 20.0f);
	graphRect.y += textPreviewHeight + 10.0f;
	graphRect.height -= textPreviewHeight + 10.0f;
	float deltaX = graphRect.width / sampleValues.size();

	//graph
	HorseRadish::OpenGL::glEnable(GL_LINE_SMOOTH);

	unsigned int samplePosIndex = 0;
	assert(GraphStatIds.size() == samplePos.size());

	for (const auto& statGraph : GraphStatIds)
	{
		auto numSamples = mProfiler.getLastQuantizedSamples(statGraph, true, sampleValues.data(), sampleValues.size());
		if (numSamples <= 1)
			continue;

		samplePos[samplePosIndex].statId = statGraph;
		samplePos[samplePosIndex].value = mProfiler.getLastQuantizedSample(statGraph);
		samplePos[samplePosIndex].posY = sampleValues[0] * graphRect.height + graphRect.y;
		samplePosIndex++;

		auto color = retrieveColor(statGraph);

		glImmediateMode.beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::LineStrip);
		glImmediateMode.setColorF(color.r, color.g, color.b);
		
		float posX = graphRect.x + graphRect.width;
		for (unsigned int i = 0; i < numSamples; i++)
		{
			glImmediateMode.addPosition(posX, sampleValues[i] * graphRect.height + graphRect.y);
			posX -= deltaX;			
		}
		
		glImmediateMode.endDraw();
	}

	HorseRadish::OpenGL::glDisable(GL_LINE_SMOOTH);

	// legend
	guiFont.paintBegin(transformMatrix.data());

	float posX = graphRect.x;
	for (const auto& curSample : samplePos)
	{
		auto color = retrieveColor(curSample.statId);

		std::string msg;
		if (curSample.value > 1000000000)
			msg = fmt::format("{0:.2f} G", curSample.value * 0.000000001);
		else if (curSample.value > 1000000)
			msg = fmt::format("{0:.2f} M", curSample.value * 0.000001);
		else if (curSample.value > 1000)
			msg = fmt::format("{0:.2f} K", curSample.value * 0.001);
		else
			msg = fmt::format("{0:.2f}", curSample.value);

		guiFont.setColor(color.r, color.g, color.b);
		guiFont.write(graphRect.x + graphRect.width + 5.0f, curSample.posY - (guiFont.getMaxHeight() * 0.5f), msg);

		msg = fmt::format("{0}: {1}", HorseRadish::Engine::Profiler::translateStatId(curSample.statId), msg);
		guiFont.write(posX, graphRect.y + 5.0f - (textPreviewHeight + 10.0f), msg);

		posX += 5.0f;
		posX += guiFont.getTextWidth(msg);
	}

	guiFont.paintEnd();
}

void ProfilerUI::drawStatsBackground(const HorseRadish::Primitives2D::Rectangle<float>& viewRect, const HorseRadish::Matrix &transformMatrix, float bkgAlpha) const
{
	HorseRadish::OpenGL::glUseProgram(0);
	HorseRadish::OpenGL::glBindProgramPipeline(mRenderer.mShaders.drawNoTex.progPipeline.getId());
	HorseRadish::OpenGL::glProgramUniformMatrix4fv(mRenderer.mShaders.drawNoTex.progVertex.getId(), mRenderer.mShaders.drawNoTex.progVertex.getUniformLocation("transformationMatrix"), 1, false, transformMatrix.data());

	auto& glImmediateMode = mRenderer.mGlImmediateMode;
	auto& guiFont = *mRenderer.mGui.font;

	glImmediateMode.beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
	glImmediateMode.setColor(0, 0, 0, HorseRadish::Color::convertColor(bkgAlpha));
	glImmediateMode.addQuad(viewRect.x, viewRect.y, viewRect.width, viewRect.height);
	glImmediateMode.endDraw();

	glImmediateMode.beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Lines);
	glImmediateMode.setColor(128, 128, 128);
	glImmediateMode.addLineV(viewRect.x, viewRect.y, viewRect.y + viewRect.height);
	glImmediateMode.addLineH(viewRect.x, viewRect.x + viewRect.width, viewRect.y);
	glImmediateMode.addLineV(viewRect.x + viewRect.width, viewRect.y, viewRect.y + viewRect.height);
	glImmediateMode.addLineH(viewRect.x, viewRect.x + viewRect.width, viewRect.y + viewRect.height);
	glImmediateMode.endDraw();
}

ProfilerUI::ProfilerUI(const HorseRadish::Engine::Profiler& profiler, HorseRadish::Render::Renderer2D& renderer)
	: mShowStats(false), mShowInfo(false), mRenderer(renderer), mProfiler(profiler)
{
	
}

void ProfilerUI::draw(const HorseRadish::OpenGL::Tools::Viewport& viewport) const
{
	if (!isVisible())
		return;

	HorseRadish::Matrix transformMatrix = viewport.getProjection(HorseRadish::OpenGL::Tools::Viewport::ProjectionType::Proj2D);

	if (mShowInfo)
	{
		HorseRadish::Primitives2D::Rectangle<float> viewRect(0.0f, 0.0f, viewport.getWidth(), viewport.getHeight());
		drawInfo(viewRect, transformMatrix);
	}

	//if (mShowStats)
	{
		HorseRadish::Primitives2D::Rectangle<float> viewRect(20.0f, 20.0f, viewport.getWidth() - 40.0f, (viewport.getHeight() * 0.5f) - 40.0f);

		drawStatsBackground(viewRect, transformMatrix, 0.8f);
		drawStats(viewRect, transformMatrix);
	}
}

void ProfilerUI::processStats()
{
	auto elapsedTime = mTimer.getTimeIntMS();
	if (elapsedTime < 1000)
		return;

	double frameTotal = mProfiler.getLastQuantizedSamples(HorseRadish::Engine::Profiler::StatId::FrameTotal, std::chrono::milliseconds(1000));
	double frameGPU = mProfiler.getLastQuantizedSamples(HorseRadish::Engine::Profiler::StatId::FrameGPU, std::chrono::milliseconds(1000));
	double frameLogic = mProfiler.getLastQuantizedSamples(HorseRadish::Engine::Profiler::StatId::FrameLogic, std::chrono::milliseconds(1000));
	double frameDraw = mProfiler.getLastQuantizedSamples(HorseRadish::Engine::Profiler::StatId::FrameDraw, std::chrono::milliseconds(1000));

	unsigned int fps = static_cast<unsigned int>(1000.0 / frameTotal);
	unsigned int gpuPercent = static_cast<unsigned int>((frameGPU / frameTotal) * 100.0);
	unsigned int cpuPercent = static_cast<unsigned int>(((frameLogic + frameDraw) / frameTotal) * 100.0);

	mInfoStr = fmt::format("FPS: {0} (CPU {1}%, GPU {2}%)", fps, gpuPercent, cpuPercent);

	mTimer.reStart();
}

bool ProfilerUI::isVisible() const
{
	return (mShowStats || mShowInfo);
}

void ProfilerUI::setStatsState(bool enabled)
{
	mShowStats = enabled;
}

void ProfilerUI::setInfoState(bool enabled)
{
	mShowInfo = enabled;
}

} }
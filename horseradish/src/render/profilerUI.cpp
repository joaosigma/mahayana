#include "profilerUI.hpp"

#include "../common/color.hpp"

#include <libs/cppformat/format.h>

namespace hr { namespace render
{
	std::array<hr::engine::Profiler::StatId, 3> GraphStatIds = {
		hr::engine::Profiler::StatId::FrameTotal,
		hr::engine::Profiler::StatId::GPUSamples,
		hr::engine::Profiler::StatId::GPUPrimitivesSubmitted
	};

	static
	hr::Color retrieveColor(hr::engine::Profiler::StatId statId)
	{
		const unsigned char *targetColor;

		switch (statId)
		{
		case hr::engine::Profiler::StatId::FrameTotal:
			targetColor = hr::Color::KnownColors::CadetBlue;
			break;
		case hr::engine::Profiler::StatId::FrameLogic:
			targetColor = hr::Color::KnownColors::BurlyWood;
			break;
		case hr::engine::Profiler::StatId::FrameDraw:
			targetColor = hr::Color::KnownColors::Firebrick;
			break;
		case hr::engine::Profiler::StatId::FrameGPU:
			targetColor = hr::Color::KnownColors::Coral;
			break;
		case hr::engine::Profiler::StatId::GPUTimeElapsed:
			targetColor = hr::Color::KnownColors::DarkSalmon;
			break;
		case hr::engine::Profiler::StatId::GPUSamples:
			targetColor = hr::Color::KnownColors::OliveDrab;
			break;
		case hr::engine::Profiler::StatId::GPUVerticesSubmitted:
			targetColor = hr::Color::KnownColors::Wheat;
			break;
		case hr::engine::Profiler::StatId::GPUPrimitivesSubmitted:
			targetColor = hr::Color::KnownColors::Sienna;
			break;
		case hr::engine::Profiler::StatId::GPUVertexShaderInvocations:
			targetColor = hr::Color::KnownColors::PeachPuff;
			break;
		case hr::engine::Profiler::StatId::GPUFragmentShaderInvocations:
			targetColor = hr::Color::KnownColors::BlanchedAlmond;
			break;
		case hr::engine::Profiler::StatId::GPUClipInputPrimitives:
			targetColor = hr::Color::KnownColors::Cornsilk;
			break;
		case hr::engine::Profiler::StatId::GPUClipOutputPrimitives:
			targetColor = hr::Color::KnownColors::MediumAquamarine;
			break;
		case hr::engine::Profiler::StatId::MemTotal:
			targetColor = hr::Color::KnownColors::Lavender;
			break;
		case hr::engine::Profiler::StatId::MemScripts:
			targetColor = hr::Color::KnownColors::Tan;
			break;
		case hr::engine::Profiler::StatId::MemObjects:
			targetColor = hr::Color::KnownColors::Plum;
			break;
		case hr::engine::Profiler::StatId::MemAnimations:
			targetColor = hr::Color::KnownColors::DodgerBlue;
			break;
		default:
			targetColor = hr::Color::KnownColors::DarkMagenta;
			break;
		}

		hr::Color color;
		color.set(targetColor);

		return color;
	}

	void ProfilerUI::drawInfo(const hr::Rectangle<float>& viewRect, const hr::Matrix &transformMatrix) const
	{
		if (mInfoStr.empty())
			return;

		auto& glImmediateMode = mRenderer.mGlImmediateMode;
		auto& guiFont = *mRenderer.mGui.font;

		float posX = viewRect.width - guiFont.getTextWidth(mInfoStr) - 10.0f;
		float posY = viewRect.height - guiFont.getMaxHeight() - 15.0f;

		hr::gl::glBindProgramPipeline(mRenderer.mShaders.drawNoTex.progPipeline.id());
		hr::gl::glProgramUniformMatrix4fv(mRenderer.mShaders.drawNoTex.progVertex.id(), mRenderer.mShaders.drawNoTex.progVertex.getUniformLocation("transformationMatrix"), 1, false, transformMatrix.data());

		glImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::LineStrip);
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

	void ProfilerUI::drawStats(const hr::Rectangle<float>& viewRect, const hr::Matrix &transformMatrix) const
	{
		if (GraphStatIds.empty())
			return;

		struct SampleLastValue
		{
			float posY;
			double value;
			hr::engine::Profiler::StatId statId;
		};

		std::array<SampleLastValue, GraphStatIds.size()> samplePos;
		std::array<double, hr::engine::Profiler::QuantizationSamples> sampleValues;

		auto& glImmediateMode = mRenderer.mGlImmediateMode;
		auto& guiFont = *mRenderer.mGui.font;

		auto textPreviewHeight = guiFont.getMaxHeight();
		auto textPreviewWidth = guiFont.getTextWidth("XXX.XX X") + 5.0f;

		hr::Rectangle<float> graphRect(viewRect.x + 10.0f, viewRect.y + 10.0f, viewRect.width - 20.0f - textPreviewWidth, viewRect.height - 20.0f);
		graphRect.y += textPreviewHeight + 10.0f;
		graphRect.height -= textPreviewHeight + 10.0f;
		float deltaX = graphRect.width / sampleValues.size();

		//graph
		hr::gl::glEnable(GL_LINE_SMOOTH);

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

			glImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::LineStrip);
			glImmediateMode.setColorF(color[0], color[1], color[2]);
			
			float posX = graphRect.x + graphRect.width;
			for (unsigned int i = 0; i < numSamples; i++)
			{
				glImmediateMode.addPosition(posX, sampleValues[i] * graphRect.height + graphRect.y);
				posX -= deltaX;			
			}
			
			glImmediateMode.endDraw();
		}

		hr::gl::glDisable(GL_LINE_SMOOTH);

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

			guiFont.setColor(color[0], color[1], color[2]);
			guiFont.write(graphRect.x + graphRect.width + 5.0f, curSample.posY - (guiFont.getMaxHeight() * 0.5f), msg);

			msg = fmt::format("{0}: {1}", hr::engine::Profiler::translateStatId(curSample.statId), msg);
			guiFont.write(posX, graphRect.y + 5.0f - (textPreviewHeight + 10.0f), msg);

			posX += 5.0f;
			posX += guiFont.getTextWidth(msg);
		}

		guiFont.paintEnd();
	}

	void ProfilerUI::drawStatsBackground(const hr::Rectangle<float>& viewRect, const hr::Matrix &transformMatrix, float bkgAlpha) const
	{
		hr::gl::glBindProgramPipeline(mRenderer.mShaders.drawNoTex.progPipeline.id());
		hr::gl::glProgramUniformMatrix4fv(mRenderer.mShaders.drawNoTex.progVertex.id(), mRenderer.mShaders.drawNoTex.progVertex.getUniformLocation("transformationMatrix"), 1, false, transformMatrix.data());

		auto& glImmediateMode = mRenderer.mGlImmediateMode;
		auto& guiFont = *mRenderer.mGui.font;

		glImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::Quads);
		glImmediateMode.setColor(0, 0, 0, hr::Color::convertColor(bkgAlpha));
		glImmediateMode.addQuad(viewRect.x, viewRect.y, viewRect.width, viewRect.height);
		glImmediateMode.endDraw();

		glImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::Lines);
		glImmediateMode.setColor(128, 128, 128);
		glImmediateMode.addLineV(viewRect.x, viewRect.y, viewRect.y + viewRect.height);
		glImmediateMode.addLineH(viewRect.x, viewRect.x + viewRect.width, viewRect.y);
		glImmediateMode.addLineV(viewRect.x + viewRect.width, viewRect.y, viewRect.y + viewRect.height);
		glImmediateMode.addLineH(viewRect.x, viewRect.x + viewRect.width, viewRect.y + viewRect.height);
		glImmediateMode.endDraw();
	}

	void ProfilerUI::draw(const hr::gl::tools::Viewport& viewport) const
	{
		if (!isVisible())
			return;

		hr::Matrix transformMatrix = viewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj2D);

		if (mShowInfo)
		{
			hr::Rectangle<float> viewRect(0.0f, 0.0f, viewport.width(), viewport.height());
			drawInfo(viewRect, transformMatrix);
		}

		if (mShowStats)
		{
			hr::Rectangle<float> viewRect(20.0f, 20.0f, viewport.width() - 40.0f, (viewport.height() * 0.5f) - 40.0f);

			drawStatsBackground(viewRect, transformMatrix, 0.8f);
			drawStats(viewRect, transformMatrix);
		}
	}

	void ProfilerUI::processStats()
	{
		auto elapsedTime = mTimer.getTimeIntMS();
		if (elapsedTime < 1000)
			return;

		double frameTotal = mProfiler.getLastQuantizedSamples(hr::engine::Profiler::StatId::FrameTotal, std::chrono::milliseconds(1000));
		double frameGPU = mProfiler.getLastQuantizedSamples(hr::engine::Profiler::StatId::FrameGPU, std::chrono::milliseconds(1000));
		double frameLogic = mProfiler.getLastQuantizedSamples(hr::engine::Profiler::StatId::FrameLogic, std::chrono::milliseconds(1000));
		double frameDraw = mProfiler.getLastQuantizedSamples(hr::engine::Profiler::StatId::FrameDraw, std::chrono::milliseconds(1000));

		auto fps = static_cast<unsigned int>(1000.0 / frameTotal);
		auto gpuPercent = static_cast<unsigned int>((frameGPU / frameTotal) * 100.0);
		auto cpuPercent = static_cast<unsigned int>(((frameLogic + frameDraw) / frameTotal) * 100.0);

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

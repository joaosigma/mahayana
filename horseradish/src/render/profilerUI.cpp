#include "profilerUI.hpp"

#include "../common/color.hpp"

#include <format>

namespace hr { namespace render
{
	std::array<hr::engine::Profiler::StatId, 3> GraphStatIds = {
		hr::engine::Profiler::StatId::FrameTotal,
		hr::engine::Profiler::StatId::GPUSamples,
		hr::engine::Profiler::StatId::GPUPrimitivesSubmitted
	};

	static
	hr::Colorf retrieveColor(hr::engine::Profiler::StatId statId)
	{
		const unsigned char *targetColor;

		switch (statId)
		{
		case hr::engine::Profiler::StatId::FrameTotal:
			targetColor = hr::Colorf::KnownColors::CadetBlue;
			break;
		case hr::engine::Profiler::StatId::FrameLogic:
			targetColor = hr::Colorf::KnownColors::BurlyWood;
			break;
		case hr::engine::Profiler::StatId::FrameDraw:
			targetColor = hr::Colorf::KnownColors::Firebrick;
			break;
		case hr::engine::Profiler::StatId::FrameGPU:
			targetColor = hr::Colorf::KnownColors::Coral;
			break;
		case hr::engine::Profiler::StatId::GPUTimeElapsed:
			targetColor = hr::Colorf::KnownColors::DarkSalmon;
			break;
		case hr::engine::Profiler::StatId::GPUSamples:
			targetColor = hr::Colorf::KnownColors::OliveDrab;
			break;
		case hr::engine::Profiler::StatId::GPUVerticesSubmitted:
			targetColor = hr::Colorf::KnownColors::Wheat;
			break;
		case hr::engine::Profiler::StatId::GPUPrimitivesSubmitted:
			targetColor = hr::Colorf::KnownColors::Sienna;
			break;
		case hr::engine::Profiler::StatId::GPUVertexShaderInvocations:
			targetColor = hr::Colorf::KnownColors::PeachPuff;
			break;
		case hr::engine::Profiler::StatId::GPUFragmentShaderInvocations:
			targetColor = hr::Colorf::KnownColors::BlanchedAlmond;
			break;
		case hr::engine::Profiler::StatId::GPUClipInputPrimitives:
			targetColor = hr::Colorf::KnownColors::Cornsilk;
			break;
		case hr::engine::Profiler::StatId::GPUClipOutputPrimitives:
			targetColor = hr::Colorf::KnownColors::MediumAquamarine;
			break;
		case hr::engine::Profiler::StatId::MemTotal:
			targetColor = hr::Colorf::KnownColors::Lavender;
			break;
		case hr::engine::Profiler::StatId::MemScripts:
			targetColor = hr::Colorf::KnownColors::Tan;
			break;
		case hr::engine::Profiler::StatId::MemObjects:
			targetColor = hr::Colorf::KnownColors::Plum;
			break;
		case hr::engine::Profiler::StatId::MemAnimations:
			targetColor = hr::Colorf::KnownColors::DodgerBlue;
			break;
		default:
			targetColor = hr::Colorf::KnownColors::DarkMagenta;
			break;
		}

		hr::Colorf color;
		color.set(targetColor);

		return color;
	}

	void ProfilerUI::drawInfo(size_t textSize, const hr::Rectangle<float>& viewRect, const hr::Matrix4f& transformMatrix) const
	{
		if (mInfoStr.empty())
			return;

		auto& glImmediateMode = mRenderer.mGlImmediateMode;
		auto& guiFont = *mRenderer.mGui.font;

		float posX = viewRect.width - guiFont.getTextWidth(textSize, mInfoStr) - 10.0f;
		float posY = viewRect.height - guiFont.getMaxHeight(textSize) - 15.0f;

		hr::gl::glBindProgramPipeline(mRenderer.mShaders.drawNoTex.progPipeline.id());
		hr::gl::glProgramUniformMatrix4fv(mRenderer.mShaders.drawNoTex.progVertex.id(), mRenderer.mShaders.drawNoTex.progVertex.getUniformLocation("transformationMatrix"), 1, false, transformMatrix.data().data());

		glImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::LineStrip);
			glImmediateMode.setColor(128, 128, 128);
			glImmediateMode.addPosition(viewRect.width, posY + guiFont.getMaxHeight(textSize) + 5.0f);
			glImmediateMode.addPosition(posX - 5.0f, posY + guiFont.getMaxHeight(textSize) + 5.0f);
			glImmediateMode.addPosition(posX - 5.0f, posY - 5.0f);
			glImmediateMode.addPosition(viewRect.width, posY - 5.0f);
		glImmediateMode.endDraw();

		guiFont.paintBegin(textSize, transformMatrix.data().data());
			guiFont.setColor(1.0f, 1.0f, 1.0f);
			guiFont.write(posX, posY, mInfoStr);
		guiFont.paintEnd();
	}

	void ProfilerUI::drawStats(size_t textSize, const hr::Rectangle<float>& viewRect, const hr::Matrix4f& transformMatrix) const
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

		auto textPreviewHeight = guiFont.getMaxHeight(textSize);
		auto textPreviewWidth = guiFont.getTextWidth(textSize, "XXX.XX X") + 5.0f;

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
		guiFont.paintBegin(textSize, transformMatrix.data().data());

		float posX = graphRect.x;
		for (const auto& curSample : samplePos)
		{
			auto color = retrieveColor(curSample.statId);

			std::string msg;
			if (curSample.value > 1000000000)
				msg = std::format("{0:.2f} G", curSample.value * 0.000000001);
			else if (curSample.value > 1000000)
				msg = std::format("{0:.2f} M", curSample.value * 0.000001);
			else if (curSample.value > 1000)
				msg = std::format("{0:.2f} K", curSample.value * 0.001);
			else
				msg = std::format("{0:.2f}", curSample.value);

			guiFont.setColor(color[0], color[1], color[2]);
			guiFont.write(graphRect.x + graphRect.width + 5.0f, curSample.posY - (guiFont.getMaxHeight(textSize) * 0.5f), msg);

			msg = std::format("{0}: {1}", hr::engine::Profiler::translateStatId(curSample.statId), msg);
			guiFont.write(posX, graphRect.y + 5.0f - (textPreviewHeight + 10.0f), msg);

			posX += 5.0f;
			posX += guiFont.getTextWidth(textSize, msg);
		}

		guiFont.paintEnd();
	}

	void ProfilerUI::drawStatsBackground(const hr::Rectangle<float>& viewRect, const hr::Matrix4f& transformMatrix, float bkgAlpha) const
	{
		hr::gl::glBindProgramPipeline(mRenderer.mShaders.drawNoTex.progPipeline.id());
		hr::gl::glProgramUniformMatrix4fv(mRenderer.mShaders.drawNoTex.progVertex.id(), mRenderer.mShaders.drawNoTex.progVertex.getUniformLocation("transformationMatrix"), 1, false, transformMatrix.data().data());

		auto& glImmediateMode = mRenderer.mGlImmediateMode;

		glImmediateMode.beginDraw(hr::gl::tools::ImmediateMode::GeometryType::Quads);
		glImmediateMode.setColor(0, 0, 0, hr::Colorf::convertColor(bkgAlpha));
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

	void ProfilerUI::draw(size_t textSize, const hr::gl::tools::Viewport& viewport) const
	{
		if (!isVisible())
			return;

		hr::Matrix4f transformMatrix = viewport.getProjection(hr::gl::tools::Viewport::ProjectionType::Proj2D);

		if (mShowInfo)
		{
			hr::Rectangle<float> viewRect(0.0f, 0.0f, viewport.width(), viewport.height());
			drawInfo(textSize, viewRect, transformMatrix);
		}

		if (mShowStats)
		{
			hr::Rectangle<float> viewRect(20.0f, 20.0f, viewport.width() - 40.0f, (viewport.height() * 0.5f) - 40.0f);

			drawStatsBackground(viewRect, transformMatrix, 0.8f);
			drawStats(textSize, viewRect, transformMatrix);
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

		mInfoStr = std::format("FPS: {0} (CPU {1}%, GPU {2}%)", fps, gpuPercent, cpuPercent);

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

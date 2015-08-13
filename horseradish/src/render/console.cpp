#include "console.hpp"

namespace HorseRadish { namespace Render {

static std::string PromptDefault = "console@main# "; //ASCII only
static const unsigned int PromptMaxSize = 1024;

void Console::updateCursorOffset(int offset)
{
	mCursor.offset = offset;
	if (mCursor.offset < 0)
		mCursor.offset = 0;
	if (mCursor.offset > mPrompt.promptUnicode.size())
		mCursor.offset = mPrompt.promptUnicode.size();

	mCursor.isVisivel = true;
	mCursor.timer.reStart();
}

void Console::processMsgPrompt(const Window::Message &msg)
{
	if (msg.getType() == Window::Message::MessageType::VirtualKey)
	{
		auto virtualKey = static_cast<Window::VirtualKeys>(msg.getParam());

		if ((msg.getFlags().piecesShort.short1 & static_cast<int>(Window::Message::MessageFlags::ControlKey)) != 0)
		{
			if (virtualKey == Window::VirtualKeys::Left && (mCursor.offset > 0))
			{
				int newOffset = mCursor.offset;
				for (newOffset--; newOffset > 0; newOffset--)
				{
					if (mPrompt.promptUnicode[newOffset - 1] == ' ' && mPrompt.promptUnicode[newOffset] != ' ')
						break;
				}

				updateCursorOffset(newOffset);
				return;
			}
			if (virtualKey == Window::VirtualKeys::Right && (mCursor.offset < mPrompt.promptUnicode.size()))
			{
				int newOffset = mCursor.offset;
				for (newOffset++; newOffset < mPrompt.promptUnicode.size(); newOffset++)
				{
					if (mPrompt.promptUnicode[newOffset - 1] == ' ' && mPrompt.promptUnicode[newOffset] != ' ')
						break;
				}

				updateCursorOffset(newOffset);
				return;
			}

			return;
		}

		switch (virtualKey)
		{
		case Window::VirtualKeys::Left:
			updateCursorOffset(mCursor.offset - 1);
			break;
		case Window::VirtualKeys::Right:
			updateCursorOffset(mCursor.offset + 1);
			break;

		case Window::VirtualKeys::Home:
			updateCursorOffset(0);
			break;
		case Window::VirtualKeys::End:
			updateCursorOffset(mPrompt.promptUnicode.size());
			break;

		case Window::VirtualKeys::Delete:
			if (mCursor.offset < mPrompt.promptUnicode.size())
				mPrompt.promptUnicode.erase(mPrompt.promptUnicode.begin() + mCursor.offset);
			updateCursorOffset(mCursor.offset);
			break;
		}
	}
	else if (msg.getType() == Window::Message::MessageType::CharacterKey)
	{
		auto keyVal = msg.getParam();

		if (keyVal == '#' || keyVal == 9 || keyVal == '\\' || keyVal == 27)
			return;

		if (keyVal == 22) //paste
		{
			std::string strUTF8;

			if (HorseRadish::Platform::ClipboardGetStrings([&](const std::string& curString) -> bool
			{
				strUTF8 = curString;
				return false;
			}) == false)
			{
				HorseRadish::Platform::ClipboardGetFiles([&](const std::string& curString) -> bool
				{
					strUTF8 = curString;
					return false;
				});
			}

			if (!strUTF8.empty() && ((mPrompt.promptUnicode.size() + strUTF8.size()) < PromptMaxSize))
			{
				HorseRadish::StringUtils::utf8Wrapper utf8Wrapper(strUTF8);

				mPrompt.promptUnicode.insert(mPrompt.promptUnicode.begin() + mCursor.offset, utf8Wrapper.begin(), utf8Wrapper.end());
				updateCursorOffset(mCursor.offset + strUTF8.size());
			}

			return;
		}

		if (keyVal == 8)
		{
			if (mPrompt.promptUnicode.empty() || (mCursor.offset == 0))
				return;

			updateCursorOffset(mCursor.offset - 1);
			mPrompt.promptUnicode.erase(mPrompt.promptUnicode.begin() + mCursor.offset);
			return;
		}

		if (mPrompt.promptUnicode.size() >= PromptMaxSize)
			return;

		mPrompt.promptUnicode.insert(mPrompt.promptUnicode.begin() + mCursor.offset, keyVal);
		updateCursorOffset(mCursor.offset + 1);
	}
}

void Console::drawContent(const HorseRadish::Matrix &transformMatrix) const
{
	if (!mRenderer.mGui.font)
		return;

	auto& guiFont = mRenderer.mGui.font;

	float maxLineY = mTextRect.y + mTextRect.height;
	float curLineY = mTextRect.y;
	auto textHeight = guiFont->getMaxHeight() * 1.1f;

	//draw prompt
	{
		guiFont->paintBegin(transformMatrix.data());
		guiFont->setColor(1.0f, 1.0f, 1.0f);

		std::string unicodeStr = PromptDefault;
		HorseRadish::StringUtils::conv2UTF8(mPrompt.promptUnicode, unicodeStr);

		auto finalLineY = curLineY;
		guiFont->layout(unicodeStr, mTextRect.width, [&](unsigned int curLine, unsigned int unicodeCharOffset, unsigned int unicodeCharCount)
		{
			auto lineY = curLineY + static_cast<float>(curLine)* textHeight;
			finalLineY += textHeight;

			if ((lineY + textHeight) >= maxLineY)
				return;

			guiFont->write(mTextRect.x, lineY, unicodeStr, unicodeCharOffset, unicodeCharCount);

			if (!mCursor.isVisivel)
				return;

			auto cursorOffset = mCursor.offset + PromptDefault.size();
			if ((cursorOffset >= unicodeCharOffset) && (cursorOffset <= (unicodeCharOffset + unicodeCharCount)))
			{
				auto strLength = guiFont->getTextWidth(unicodeStr, unicodeCharOffset, cursorOffset - unicodeCharOffset);
				guiFont->writeChar(mTextRect.x + strLength, lineY, '_');

				auto cursorChar = HorseRadish::StringUtils::getUnicodeAt(unicodeStr, cursorOffset);
				if (cursorChar != '\0' && cursorChar != ' ')
				{
					guiFont->setColor(1.0f, 0.0f, 0.0f);
					guiFont->writeChar(mTextRect.x + strLength, lineY, cursorChar);
					guiFont->setColor(1.0f, 1.0f, 1.0f);
				}
			}
		});

		curLineY = finalLineY;

		guiFont->paintEnd();
	}

	curLineY += (textHeight * 0.5f);

	//draw logger history
	{
		struct LogMsg
		{
			Engine::Logger::EntryType entryType;
			bool isMsgFormatted;
			std::string msg;
		};
		std::vector<LogMsg> logMsgs;

		int maxLines = Math::ftoi((maxLineY - curLineY) / textHeight);
		if (maxLines <= 0)
			return;

		logMsgs.reserve(maxLines);
		mLogger.iterateBuffer([&](const Engine::Logger::EntryType entryType, const Engine::Logger::ModuleType moduleType, const bool isMsgFormatted, const std::string& msg)
		{
			logMsgs.push_back({ entryType, isMsgFormatted,  msg });

			maxLines--;
			return (maxLines > 0);
		}, mLogView.offset);

		guiFont->paintBegin(transformMatrix.data());
		guiFont->setColor(1.0f, 1.0f, 1.0f);

		for (const auto& logMsg : logMsgs)
		{
			if ((curLineY + textHeight) >= maxLineY)
				break;

			if (logMsg.msg.empty())
				continue;

			switch (logMsg.entryType)
			{
			case Engine::Logger::EntryType::Error:
				guiFont->setColor(1.0f, 0.0f, 0.0f, 1.0f);
				break;
			case Engine::Logger::EntryType::Warning:
				guiFont->setColor(1.0f, 0.42f, 0.17f, 1.0f);
				break;
			default:
				guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);
				break;
			}

			if (!logMsg.isMsgFormatted)
			{
				auto finalLineY = curLineY;
				guiFont->layout(logMsg.msg, mTextRect.width, [&](unsigned int curLine, unsigned int unicodeCharOffset, unsigned int unicodeCharCount)
				{
					auto lineY = curLineY + static_cast<float>(curLine) * textHeight;
					finalLineY += textHeight;

					if ((lineY + textHeight) >= maxLineY)
						return;

					guiFont->write(mTextRect.x, lineY, logMsg.msg, unicodeCharOffset, unicodeCharCount);
				});

				curLineY = finalLineY;
				continue;
			}

			auto curX = mTextRect.x;

			unsigned int lastChar = 0;
			HorseRadish::StringUtils::utf8Wrapper textWrapper(logMsg.msg);
			for (HorseRadish::StringUtils::utf8Wrapper::const_iterator it = textWrapper.begin(), itEnd = textWrapper.end(); it != itEnd; ++it)
			{
				auto curChar = *it;

				HorseRadish::StringUtils::utf8Wrapper::const_iterator itNext(it);
				itNext++;

				if ((curChar == '$') && (lastChar != '$'))
				{
					HorseRadish::StringUtils::utf8Wrapper::const_iterator itNext(it);
					itNext++;

					if ((*itNext == '{'))
					{
						it++;
						it++;

						std::string value;
						value.reserve(10);

						for (; (*it != '}') && (it != itEnd); it++)
							value += *it;

						HorseRadish::StringUtils::trim(value);
						if (value.empty())
							continue;

						if (value[0] == '#')
							guiFont->setColor(Color::parseColorFromHTML(value.c_str()));
						else if (value == "red")
							guiFont->setColor(1.0f, 0.0f, 0.0f, 1.0f);
						else if (value == "green")
							guiFont->setColor(0.0f, 1.0f, 0.0f, 1.0f);
						else if (value == "bgreen")
							guiFont->setColor(0.08f, 1.0f, 0.39f, 1.0f);
						else if (value == "olive")
							guiFont->setColor(0.59f, 0.59f, 0.0f, 1.0f);
						else if (value == "default")
							guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);

						continue;
					}
				}

				lastChar = *it;
				curX += guiFont->writeChar(curX, curLineY, lastChar);
			}

			guiFont->setColor(1.0f, 1.0f, 1.0f, 1.0f);
			curLineY += textHeight;
		}

		guiFont->paintEnd();
	}
}

void Console::drawBackground(const HorseRadish::Matrix &transformMatrix, float bkgAlpha) const
{
	HorseRadish::OpenGL::glUseProgram(0);
	HorseRadish::OpenGL::glBindProgramPipeline(mRenderer.mShaders.drawNoTex.progPipeline.getId());
	HorseRadish::OpenGL::glProgramUniformMatrix4fv(mRenderer.mShaders.drawNoTex.progVertex.getId(), mRenderer.mShaders.drawNoTex.progVertex.getUniformLocation("transformationMatrix"), 1, false, transformMatrix.data());

	auto& glImmediateMode = mRenderer.mGlImmediateMode;
	auto& guiFont = mRenderer.mGui.font;

	glImmediateMode->beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Quads);
		glImmediateMode->setColor(0, 0, 0, HorseRadish::Color::convertColor(bkgAlpha));
		glImmediateMode->addQuad(mViewRect.x, mViewRect.y, mViewRect.width, mViewRect.height);
	glImmediateMode->endDraw();

	glImmediateMode->beginDraw(HorseRadish::OpenGL::Tools::ImmediateMode::GeometryType::Lines);
		glImmediateMode->setColor(128, 128, 128);
		glImmediateMode->addLineV(mViewRect.x, mViewRect.y, mViewRect.y + mViewRect.height);
		glImmediateMode->addLineH(mViewRect.x, mViewRect.x + mViewRect.width, mViewRect.y);
		glImmediateMode->addLineV(mViewRect.x + mViewRect.width, mViewRect.y, mViewRect.y + mViewRect.height);
		glImmediateMode->addLineH(mViewRect.x, mViewRect.x + mViewRect.width, mViewRect.y + mViewRect.height);
	glImmediateMode->endDraw();
}

Console::Console(const HorseRadish::Engine::Logger& logger, HorseRadish::Render::Renderer2D& renderer, unsigned int maxPromptHistory)
	: mMainVisible(false), mRenderer(renderer), mLogger(logger)
{
	mLogView.offset = 0;
	mCursor.offset = 0;
	mCursor.isVisivel = false;
	mPrompt.historyOffset = 0;
	mPrompt.maxHistorySize = maxPromptHistory;

	mViewRect.Set(30.0f, 30.0f, mRenderer.mRenderWidth - 60.0f, mRenderer.mRenderHeight - 60.0f);
	mTextRect.Set(mViewRect.x + 9.0f, mViewRect.y + 9.0f, mViewRect.width - 18.0f, mViewRect.height - 18.0f);
}

void Console::draw(const HorseRadish::OpenGL::Tools::Viewport& viewport) const
{
	if (!isVisible())
		return;

	HorseRadish::Matrix transformMatrix = viewport.getProjection(HorseRadish::OpenGL::Tools::Viewport::ProjectionType::Proj2D);

	drawBackground(transformMatrix, 0.6f);
	drawContent(transformMatrix);
}

bool Console::isVisible() const
{
	return (mMainVisible || !mAlerts.empty());
}

void Console::setVisible(bool visible)
{
	mMainVisible = visible;
}

void Console::processStep()
{
	if (mCursor.timer.getTimeMS() > 750.0f)
	{
		mCursor.isVisivel = !mCursor.isVisivel;
		mCursor.timer.reStart();
	}
}

void Console::processMsg(std::function<void(const char * const)> execPromptCmdCb, const Window::Message &msg)
{
	if (isVisible())
	{
		if (msg.getType() == Window::Message::MessageType::MouseWheel)
		{
			mLogView.offset += static_cast<signed short>(msg.getFlags().piecesShort.short0);
			if (mLogView.offset < 0)
				mLogView.offset = 0;

			return;
		}

		if ((msg.getType() != Window::Message::MessageType::CharacterKey) && (msg.getType() != Window::Message::MessageType::VirtualKey))
			return;

		if (msg.getType() == Window::Message::MessageType::VirtualKey)
		{
			switch (static_cast<Window::VirtualKeys>(msg.getParam()))
			{
			case Window::VirtualKeys::PageUp:
				{
					if (msg.getFlags().piecesShort.short1 & static_cast<int>(Window::Message::MessageFlags::ControlKey))
						mLogView.offset = 0;
					else
						mLogView.offset++;

					return;
				}

			case Window::VirtualKeys::PageDown:
				{
					if (msg.getFlags().piecesShort.short1 & static_cast<int>(Window::Message::MessageFlags::ControlKey))
						mLogView.offset = 0;
					else
						mLogView.offset -= (mLogView.offset > 0) ? 1 : 0;
					return;
				}

			case Window::VirtualKeys::Up:
				{
					if (mPrompt.history.empty())
						return;

					mPrompt.historyOffset++;
					if (mPrompt.historyOffset >= mPrompt.history.size())
						mPrompt.historyOffset = mPrompt.history.size() - 1;

					mPrompt.promptUnicode.clear();
					for (const auto& curUnicode : HorseRadish::StringUtils::utf8Wrapper(mPrompt.history[mPrompt.historyOffset]))
						mPrompt.promptUnicode.push_back(curUnicode);

					updateCursorOffset(mPrompt.promptUnicode.size());
					return;
				}
				
			case Window::VirtualKeys::Down:
				{
					if (mPrompt.historyOffset == 0)
						return;

					mPrompt.historyOffset--;

					mPrompt.promptUnicode.clear();
					for (const auto& curUnicode : HorseRadish::StringUtils::utf8Wrapper(mPrompt.history[mPrompt.historyOffset]))
						mPrompt.promptUnicode.push_back(curUnicode);

					updateCursorOffset(mPrompt.promptUnicode.size());
					return;
				}
			}
		}

		if (msg.getType() == Window::Message::MessageType::CharacterKey && msg.getParam() == 13) //enter
		{
			if (mPrompt.promptUnicode.empty())
				return;

			auto promptStr = HorseRadish::StringUtils::conv2UTF8(mPrompt.promptUnicode);
			mPrompt.promptUnicode.clear();
			updateCursorOffset(0);

			mPrompt.history.push_front(promptStr);
			if (mPrompt.history.size() > mPrompt.maxHistorySize)
				mPrompt.history.pop_back();

			if (execPromptCmdCb)
				execPromptCmdCb(promptStr.c_str());

			return;
		}

		//if not dealt where, send to prompt
		processMsgPrompt(msg);
	}

	if ((msg.getType() == Window::Message::MessageType::VirtualKey) && (msg.getParam() == static_cast<int>(Window::VirtualKeys::Escape)))
	{
		mMainVisible = !mMainVisible;
		return;
	}
}

} }
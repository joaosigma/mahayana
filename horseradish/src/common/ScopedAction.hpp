#pragma once

#include <functional>

namespace hr
{
	class ScopedAction
	{
		std::function<void()> funcCallback;

	public:
		ScopedAction(const ScopedAction&) = delete;
		ScopedAction& operator=(const ScopedAction&) = delete;

		explicit ScopedAction(std::function<void()> funcCallback)
			: funcCallback(funcCallback)
		{ }

		~ScopedAction()
		{
			if (funcCallback)
				funcCallback();
		}
	};
}

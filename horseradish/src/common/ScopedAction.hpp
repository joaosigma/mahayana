#pragma once
#ifndef __HSCOPED_ACTION__
#define __HSCOPED_ACTION__

#include "Types.hpp"

#include <assert.h>
#include <functional>

namespace HorseRadish
{
	class ScopedAction
	{
		std::function<void()> funcCallback;

	public:
		ScopedAction(const ScopedAction&) = delete;
		ScopedAction& operator=(const ScopedAction&) = delete;

		explicit ScopedAction(std::function<void()> funcCallback)
			: funcCallback(funcCallback)
		{
		}

		~ScopedAction()
		{
			if (funcCallback != nullptr)
				funcCallback();
		}
	};

} //namespace HorseRadish

#endif
#pragma once

#include <functional>

namespace hr
{
    class ScopedAction
    {
        std::function<void()> funcCallback;

    public:
        explicit ScopedAction(std::function<void()> funcCallback)
          : funcCallback{std::move(funcCallback)}
        {}

        ScopedAction() noexcept = default;

        ScopedAction(const ScopedAction&) = delete;
        ScopedAction& operator=(const ScopedAction&) = delete;
        ScopedAction(ScopedAction&&) = delete;
        ScopedAction& operator=(ScopedAction&&) = delete;

        ~ScopedAction()
        {
            if (funcCallback)
                funcCallback();
        }
    };
}

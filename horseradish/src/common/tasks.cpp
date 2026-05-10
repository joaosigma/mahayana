module core:tasks.impl;

import :tasks;

namespace hr
{
    std::atomic<size_t> Scheduler::sGenTaskId{0};
    std::atomic<size_t> Dispatcher::sGenDispatcherId{0};
}

#include "tasks.hpp"

namespace hr
{
    std::atomic<size_t> Scheduler::sGenTaskId{0};
    std::atomic<size_t> Dispatcher::sGenDispatcherId{0};
}

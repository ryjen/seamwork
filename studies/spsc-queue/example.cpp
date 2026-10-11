#include <cstdint>
#include <iostream>

#include <seamwork/studies/spsc_queue.hpp>

namespace sw = seamwork::studies::spsc_queue;

struct realtime_event {
    std::uint32_t id{};
    std::uint64_t timestamp{};
};

int main()
{
    sw::queue<realtime_event, 4> queue;

    if (!queue.try_push(realtime_event{.id = 7, .timestamp = 42})) {
        return 2;
    }

    const auto event = queue.try_pop();
    if (!event) {
        return 3;
    }

    std::cout << "event " << event->id
              << " @ " << event->timestamp << '\n';
}

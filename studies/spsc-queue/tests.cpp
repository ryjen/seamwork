#include <atomic>
#include <cstddef>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

#include <seamwork/studies/spsc_queue.hpp>

namespace sw = seamwork::studies::spsc_queue;

namespace {

std::atomic_bool track_allocations{false};
std::atomic_size_t allocation_count{0};

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

struct observation {
    std::uint64_t frame{};
    std::uint16_t peak{};
};

struct timestamped_event {
    std::uint8_t kind{};
    std::uint64_t timestamp{};
};

struct move_only {
    explicit move_only(int value) noexcept
        : value(value)
    {
    }

    move_only(const move_only&) = delete;
    auto operator=(const move_only&) -> move_only& = delete;

    move_only(move_only&& other) noexcept
        : value(std::exchange(other.value, -1))
    {
    }

    auto operator=(move_only&& other) noexcept -> move_only&
    {
        value = std::exchange(other.value, -1);
        return *this;
    }

    int value{};
};

static_assert(sw::queue<int, 4>::capacity == 4);
static_assert(!std::copy_constructible<sw::queue<int, 4>>);
static_assert(!std::move_constructible<sw::queue<int, 4>>);

void test_fifo_full_empty_and_refill()
{
    sw::queue<observation, 4> queue;

    require(!queue.try_pop(), "new queue should be empty");

    require(queue.try_push(observation{.frame = 1, .peak = 101}), "push 1");
    require(queue.try_push(observation{.frame = 2, .peak = 102}), "push 2");
    require(queue.try_push(observation{.frame = 3, .peak = 103}), "push 3");
    require(queue.try_push(observation{.frame = 4, .peak = 104}), "push 4");
    require(
        !queue.try_push(observation{.frame = 5, .peak = 105}),
        "push should fail without overwriting when full"
    );

    for (std::uint64_t expected = 1; expected <= 4; ++expected) {
        const auto item = queue.try_pop();
        require(item.has_value(), "full queue should drain");
        require(item->frame == expected, "queue should preserve FIFO frame order");
        require(
            item->peak == static_cast<std::uint16_t>(100 + expected),
            "queue should preserve payload values"
        );
    }

    require(!queue.try_pop(), "drained queue should be empty");

    require(queue.try_push(observation{.frame = 6, .peak = 106}), "refill push");
    const auto refilled = queue.try_pop();
    require(refilled && refilled->frame == 6, "queue should be reusable after drain");
}

void test_second_consumer_shape()
{
    sw::queue<timestamped_event, 2> queue;

    require(
        queue.try_push(timestamped_event{.kind = 7, .timestamp = 42}),
        "event push"
    );

    const auto event = queue.try_pop();
    require(event.has_value(), "event should pop");
    require(event->kind == 7, "event kind should round-trip");
    require(event->timestamp == 42, "event timestamp should round-trip");
}

void test_move_only_payload()
{
    sw::queue<move_only, 2> queue;

    move_only value{41};
    require(queue.try_push(std::move(value)), "move-only payload should push");
    require(value.value == -1, "push should consume moved value");

    const auto popped = queue.try_pop();
    require(popped.has_value(), "move-only payload should pop");
    require(popped->value == 41, "move-only payload value should survive");
}

void test_queue_operations_allocate_nothing()
{
    sw::queue<observation, 8> queue;

    allocation_count.store(0, std::memory_order_relaxed);
    track_allocations.store(true, std::memory_order_release);

    for (std::uint64_t i = 0; i < 10'000; ++i) {
        if (!queue.try_push(observation{.frame = i, .peak = 100})) {
            std::abort();
        }

        const auto item = queue.try_pop();
        if (!item || item->frame != i) {
            std::abort();
        }
    }

    track_allocations.store(false, std::memory_order_release);
    require(
        allocation_count.load(std::memory_order_relaxed) == 0,
        "queue push/pop should not allocate"
    );
}

void test_sequence_wraparound_with_small_counter()
{
    sw::detail::basic_spsc_queue<int, 4, std::uint8_t> queue;

    for (int cycle = 0; cycle < 200; ++cycle) {
        for (int offset = 0; offset < 4; ++offset) {
            require(
                queue.try_push(cycle * 4 + offset),
                "wrap push should succeed"
            );
        }
        require(!queue.try_push(-1), "wrap full check");

        for (int offset = 0; offset < 4; ++offset) {
            const auto value = queue.try_pop();
            require(value.has_value(), "wrap pop should succeed");
            require(
                *value == cycle * 4 + offset,
                "wraparound should preserve FIFO order"
            );
        }

        require(!queue.try_pop(), "wrap queue should be empty after drain");
    }
}

void test_concurrent_spsc_ordering()
{
    constexpr std::uint32_t count = 100'000;
    sw::queue<std::uint32_t, 64> queue;

    std::atomic_bool start{false};
    std::atomic_bool failed{false};

    std::thread producer{
        [&]() {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            for (std::uint32_t value = 0; value < count; ++value) {
                while (!queue.try_push(value)) {
                    if (failed.load(std::memory_order_acquire)) {
                        return;
                    }
                    std::this_thread::yield();
                }
            }
        }
    };

    std::thread consumer{
        [&]() {
            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            std::uint32_t expected = 0;
            while (expected < count) {
                const auto value = queue.try_pop();
                if (!value) {
                    std::this_thread::yield();
                    continue;
                }

                if (*value != expected) {
                    failed.store(true, std::memory_order_release);
                    return;
                }
                ++expected;
            }
        }
    };

    start.store(true, std::memory_order_release);
    producer.join();
    consumer.join();

    require(
        !failed.load(std::memory_order_acquire),
        "SPSC stress should preserve order without loss or duplication"
    );
    require(!queue.try_pop(), "queue should end empty after stress");
}

} // namespace

void* operator new(std::size_t size)
{
    if (track_allocations.load(std::memory_order_relaxed)) {
        allocation_count.fetch_add(1, std::memory_order_relaxed);
    }

    if (void* memory = std::malloc(size)) {
        return memory;
    }
    throw std::bad_alloc{};
}

void operator delete(void* memory) noexcept
{
    std::free(memory);
}

void operator delete(void* memory, std::size_t) noexcept
{
    std::free(memory);
}

void* operator new[](std::size_t size)
{
    return ::operator new(size);
}

void operator delete[](void* memory) noexcept
{
    ::operator delete(memory);
}

void operator delete[](void* memory, std::size_t) noexcept
{
    ::operator delete(memory);
}

int main()
{
    try {
        test_fifo_full_empty_and_refill();
        test_second_consumer_shape();
        test_move_only_payload();
        test_queue_operations_allocate_nothing();
        test_sequence_wraparound_with_small_counter();
        test_concurrent_spsc_ordering();
    } catch (const std::exception& error) {
        std::cerr << "SPSC queue study failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

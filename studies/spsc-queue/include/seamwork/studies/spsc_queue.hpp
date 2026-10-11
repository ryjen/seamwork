#pragma once

#include <array>
#include <atomic>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

namespace seamwork::studies::spsc_queue {

namespace detail {

template <class T, std::size_t Capacity, std::unsigned_integral Sequence>
    requires(
        Capacity > 0 &&
        std::has_single_bit(Capacity) &&
        Capacity <= static_cast<std::size_t>(std::numeric_limits<Sequence>::max()) &&
        std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_destructible_v<T>)
class basic_spsc_queue
{
public:
    static constexpr std::size_t capacity = Capacity;

    basic_spsc_queue() = default;

    basic_spsc_queue(const basic_spsc_queue&) = delete;
    auto operator=(const basic_spsc_queue&) -> basic_spsc_queue& = delete;
    basic_spsc_queue(basic_spsc_queue&&) = delete;
    auto operator=(basic_spsc_queue&&) -> basic_spsc_queue& = delete;

    [[nodiscard]] auto try_push(T&& value) noexcept -> bool
    {
        return push_impl(std::move(value));
    }

    [[nodiscard]] auto try_push(const T& value) noexcept -> bool
        requires std::is_nothrow_copy_constructible_v<T>
    {
        return push_impl(value);
    }

    [[nodiscard]] auto try_pop() noexcept -> std::optional<T>
    {
        const auto read = read_sequence_.load(std::memory_order_relaxed);
        const auto write = write_sequence_.load(std::memory_order_acquire);

        if (read == write) {
            return std::nullopt;
        }

        auto& slot = entries_[slot_index(read)];
        std::optional<T> result{std::move(*slot)};
        slot.reset();

        read_sequence_.store(next(read), std::memory_order_release);
        return result;
    }

private:
    template <class U>
    [[nodiscard]] auto push_impl(U&& value) noexcept -> bool
    {
        const auto write = write_sequence_.load(std::memory_order_relaxed);
        const auto read = read_sequence_.load(std::memory_order_acquire);

        if (distance(write, read) >= Capacity) {
            return false;
        }

        auto& slot = entries_[slot_index(write)];
        slot.emplace(std::forward<U>(value));

        write_sequence_.store(next(write), std::memory_order_release);
        return true;
    }

    [[nodiscard]] static constexpr auto next(Sequence value) noexcept -> Sequence
    {
        return static_cast<Sequence>(value + Sequence{1});
    }

    [[nodiscard]] static constexpr auto distance(
        Sequence write,
        Sequence read
    ) noexcept -> std::size_t
    {
        return static_cast<std::size_t>(static_cast<Sequence>(write - read));
    }

    [[nodiscard]] static constexpr auto slot_index(Sequence sequence) noexcept
        -> std::size_t
    {
        return static_cast<std::size_t>(sequence) % Capacity;
    }

    static_assert(std::atomic<Sequence>::is_always_lock_free);

    std::array<std::optional<T>, Capacity> entries_{};
    std::atomic<Sequence> write_sequence_{0};
    std::atomic<Sequence> read_sequence_{0};
};

} // namespace detail

template <class T, std::size_t Capacity>
using queue = detail::basic_spsc_queue<T, Capacity, std::uint32_t>;

} // namespace seamwork::studies::spsc_queue

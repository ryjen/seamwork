#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <random>
#include <span>
#include <utility>

#include <seamwork/yahtsee/domain.hpp>

namespace seamwork::studies::yahtsee {

inline constexpr std::size_t dice_per_hand = seamwork::yahtsee::dice_per_hand;
inline constexpr unsigned sides_per_die = 6;

using face = seamwork::yahtsee::face;
using face_values = seamwork::yahtsee::face_values;
using hand = seamwork::yahtsee::hand;
using seamwork::yahtsee::value;

using hold_view = std::span<const bool, dice_per_hand>;

template <class Source>
concept face_source = requires(Source& source) {
    { std::invoke(source) } -> std::same_as<face>;
};

template <face_source Source>
[[nodiscard]] auto initial_roll(Source& source) -> hand
{
    face_values values{};
    for (auto& die : values) {
        die = std::invoke(source);
    }
    return hand{values};
}

template <face_source Source>
[[nodiscard]] auto reroll(const hand& current, hold_view held, Source& source) -> hand
{
    auto values = current.values();

    for (std::size_t index = 0; index < values.size(); ++index) {
        if (!held[index]) {
            values[index] = std::invoke(source);
        }
    }

    return hand{values};
}

template <std::uniform_random_bit_generator Engine>
class uniform_face_source
{
public:
    explicit uniform_face_source(Engine engine)
        : engine_(std::move(engine))
    {
    }

    [[nodiscard]] auto operator()() -> face
    {
        std::uniform_int_distribution<unsigned> distribution{1, sides_per_die};
        return static_cast<face>(distribution(engine_));
    }

private:
    Engine engine_;
};

namespace alternatives {

class virtual_face_source
{
public:
    virtual ~virtual_face_source() = default;
    [[nodiscard]] virtual auto next() -> face = 0;
};

[[nodiscard]] inline auto initial_roll(virtual_face_source& source) -> hand
{
    face_values values{};
    for (auto& die : values) {
        die = source.next();
    }
    return hand{values};
}

[[nodiscard]] inline auto reroll(
    const hand& current,
    hold_view held,
    virtual_face_source& source
) -> hand
{
    auto values = current.values();

    for (std::size_t index = 0; index < values.size(); ++index) {
        if (!held[index]) {
            values[index] = source.next();
        }
    }

    return hand{values};
}

using erased_face_source = std::function<face()>;

[[nodiscard]] inline auto initial_roll(erased_face_source& source) -> hand
{
    face_values values{};
    for (auto& die : values) {
        die = source();
    }
    return hand{values};
}

[[nodiscard]] inline auto reroll(
    const hand& current,
    hold_view held,
    erased_face_source& source
) -> hand
{
    auto values = current.values();

    for (std::size_t index = 0; index < values.size(); ++index) {
        if (!held[index]) {
            values[index] = source();
        }
    }

    return hand{values};
}

template <std::uniform_random_bit_generator Engine>
[[nodiscard]] auto initial_roll_urbg(Engine& engine) -> hand
{
    std::uniform_int_distribution<unsigned> distribution{1, sides_per_die};
    face_values values{};

    for (auto& die : values) {
        die = static_cast<face>(distribution(engine));
    }

    return hand{values};
}

template <std::uniform_random_bit_generator Engine>
[[nodiscard]] auto reroll_urbg(const hand& current, hold_view held, Engine& engine) -> hand
{
    std::uniform_int_distribution<unsigned> distribution{1, sides_per_die};
    auto values = current.values();

    for (std::size_t index = 0; index < values.size(); ++index) {
        if (!held[index]) {
            values[index] = static_cast<face>(distribution(engine));
        }
    }

    return hand{values};
}

} // namespace alternatives

} // namespace seamwork::studies::yahtsee

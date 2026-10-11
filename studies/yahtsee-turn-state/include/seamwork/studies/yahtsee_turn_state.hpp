#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <utility>
#include <variant>

#include <seamwork/yahtsee/domain.hpp>

namespace seamwork::studies::yahtsee_turn {

inline constexpr std::size_t dice_per_hand = seamwork::yahtsee::dice_per_hand;

using hold_mask = std::array<bool, dice_per_hand>;

enum class prior_roll_count : std::uint8_t {
    zero = 0,
    one = 1,
    two = 2,
};

enum class choice_roll_count : std::uint8_t {
    one = 1,
    two = 2,
};

enum class completed_roll_count : std::uint8_t {
    one = 1,
    two = 2,
    three = 3,
};

[[nodiscard]] constexpr auto value(prior_roll_count count) noexcept -> unsigned
{
    return static_cast<unsigned>(std::to_underlying(count));
}

[[nodiscard]] constexpr auto value(choice_roll_count count) noexcept -> unsigned
{
    return static_cast<unsigned>(std::to_underlying(count));
}

[[nodiscard]] constexpr auto value(completed_roll_count count) noexcept -> unsigned
{
    return static_cast<unsigned>(std::to_underlying(count));
}

class awaiting_roll
{
public:
    constexpr explicit awaiting_roll(
        prior_roll_count rolls_taken = prior_roll_count::zero,
        hold_mask held = {}
    ) noexcept
        : rolls_taken_(rolls_taken), held_(held)
    {
    }

    [[nodiscard]] constexpr auto rolls_taken() const noexcept -> prior_roll_count
    {
        return rolls_taken_;
    }

    [[nodiscard]] constexpr auto held() const noexcept -> const hold_mask&
    {
        return held_;
    }

private:
    prior_roll_count rolls_taken_;
    hold_mask held_;
};

class choosing_dice
{
public:
    constexpr explicit choosing_dice(choice_roll_count rolls_taken, hold_mask held = {}) noexcept
        : rolls_taken_(rolls_taken), held_(held)
    {
    }

    [[nodiscard]] constexpr auto rolls_taken() const noexcept -> choice_roll_count
    {
        return rolls_taken_;
    }

    [[nodiscard]] constexpr auto held() const noexcept -> const hold_mask&
    {
        return held_;
    }

    [[nodiscard]] constexpr auto with_toggled_hold(std::size_t index) const -> choosing_dice
    {
        auto next = *this;
        next.held_[index] = !next.held_[index];
        return next;
    }

private:
    choice_roll_count rolls_taken_;
    hold_mask held_;
};

class must_score
{
public:
    constexpr explicit must_score(hold_mask held = {}) noexcept
        : held_(held)
    {
    }

    [[nodiscard]] constexpr auto held() const noexcept -> const hold_mask&
    {
        return held_;
    }

private:
    hold_mask held_;
};

class completed
{
public:
    constexpr explicit completed(completed_roll_count rolls_taken) noexcept
        : rolls_taken_(rolls_taken)
    {
    }

    [[nodiscard]] constexpr auto rolls_taken() const noexcept -> completed_roll_count
    {
        return rolls_taken_;
    }

private:
    completed_roll_count rolls_taken_;
};

using turn_state = std::variant<awaiting_roll, choosing_dice, must_score, completed>;

enum class turn_action : std::uint8_t {
    roll_completed,
    reroll,
    score,
};

struct toggle_hold {
    std::size_t index{};
};

using command = std::variant<turn_action, toggle_hold>;

enum class transition_error_code : std::uint8_t {
    invalid_action,
    die_out_of_range,
};

struct transition_error {
    transition_error_code code;
    std::size_t index{};
};

using transition_result = std::expected<turn_state, transition_error>;

[[nodiscard]] constexpr auto initial_turn() -> turn_state
{
    return awaiting_roll{};
}

[[nodiscard]] constexpr auto roll_completed(const awaiting_roll& current) -> turn_state
{
    switch (current.rolls_taken()) {
    case prior_roll_count::zero:
        return choosing_dice{choice_roll_count::one, current.held()};
    case prior_roll_count::one:
        return choosing_dice{choice_roll_count::two, current.held()};
    case prior_roll_count::two:
        return must_score{current.held()};
    }

    std::unreachable();
}

[[nodiscard]] constexpr auto reroll(const choosing_dice& current) -> turn_state
{
    const auto prior = current.rolls_taken() == choice_roll_count::one
        ? prior_roll_count::one
        : prior_roll_count::two;
    return awaiting_roll{prior, current.held()};
}

[[nodiscard]] constexpr auto score(const choosing_dice& current) -> turn_state
{
    const auto completed_rolls = current.rolls_taken() == choice_roll_count::one
        ? completed_roll_count::one
        : completed_roll_count::two;
    return completed{completed_rolls};
}

[[nodiscard]] constexpr auto score(const must_score&) -> turn_state
{
    return completed{completed_roll_count::three};
}

[[nodiscard]] constexpr auto with_toggled_hold(
    const choosing_dice& current,
    std::size_t index
) -> std::expected<choosing_dice, transition_error>
{
    if (index >= dice_per_hand) {
        return std::unexpected{transition_error{
            .code = transition_error_code::die_out_of_range,
            .index = index,
        }};
    }

    return current.with_toggled_hold(index);
}

template <class... Visitors>
struct overloaded : Visitors... {
    using Visitors::operator()...;
};

template <class... Visitors>
overloaded(Visitors...) -> overloaded<Visitors...>;

[[nodiscard]] constexpr auto invalid_transition() -> transition_result
{
    return std::unexpected{transition_error{
        .code = transition_error_code::invalid_action,
    }};
}

[[nodiscard]] constexpr auto apply(const turn_state& current, const command& requested)
    -> transition_result
{
    return std::visit(
        overloaded{
            [&](const awaiting_roll& state) -> transition_result {
                const auto* action = std::get_if<turn_action>(&requested);
                if (action != nullptr && *action == turn_action::roll_completed) {
                    return roll_completed(state);
                }
                return invalid_transition();
            },
            [&](const choosing_dice& state) -> transition_result {
                if (const auto* toggle = std::get_if<toggle_hold>(&requested); toggle != nullptr) {
                    auto next = with_toggled_hold(state, toggle->index);
                    if (!next) {
                        return std::unexpected{next.error()};
                    }
                    return turn_state{*next};
                }

                const auto* action = std::get_if<turn_action>(&requested);
                if (action == nullptr) {
                    return invalid_transition();
                }
                if (*action == turn_action::reroll) {
                    return reroll(state);
                }
                if (*action == turn_action::score) {
                    return score(state);
                }
                return invalid_transition();
            },
            [&](const must_score& state) -> transition_result {
                const auto* action = std::get_if<turn_action>(&requested);
                if (action != nullptr && *action == turn_action::score) {
                    return score(state);
                }
                return invalid_transition();
            },
            [&](const completed&) -> transition_result {
                return invalid_transition();
            },
        },
        current
    );
}

} // namespace seamwork::studies::yahtsee_turn

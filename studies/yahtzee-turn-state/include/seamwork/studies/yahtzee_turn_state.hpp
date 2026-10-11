#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <type_traits>
#include <variant>

namespace seamwork::studies::yahtzee_turn {

inline constexpr std::size_t dice_per_hand = 5;
inline constexpr unsigned max_rolls = 3;

using hold_mask = std::array<bool, dice_per_hand>;

struct awaiting_roll {
    unsigned rolls_taken{};
    hold_mask held{};
};

struct choosing_dice {
    unsigned rolls_taken{};
    hold_mask held{};
};

struct must_score {
    hold_mask held{};
};

struct completed {
    unsigned rolls_taken{};
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
    const auto completed_rolls = current.rolls_taken + 1;
    if (completed_rolls >= max_rolls) {
        return must_score{current.held};
    }

    return choosing_dice{completed_rolls, current.held};
}

[[nodiscard]] constexpr auto reroll(const choosing_dice& current) -> turn_state
{
    return awaiting_roll{current.rolls_taken, current.held};
}

[[nodiscard]] constexpr auto score(const choosing_dice& current) -> turn_state
{
    return completed{current.rolls_taken};
}

[[nodiscard]] constexpr auto score(const must_score&) -> turn_state
{
    return completed{max_rolls};
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

    auto next = current;
    next.held[index] = !next.held[index];
    return next;
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

} // namespace seamwork::studies::yahtzee_turn

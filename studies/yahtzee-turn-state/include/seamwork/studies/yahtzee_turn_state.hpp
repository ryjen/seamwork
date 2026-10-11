#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
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

[[nodiscard]] constexpr auto apply(const turn_state& current, const command& requested)
    -> transition_result
{
    return std::visit(
        [&](const auto& state) -> transition_result {
            using state_type = std::remove_cvref_t<decltype(state)>;

            return std::visit(
                [&](const auto& requested_command) -> transition_result {
                    using command_type = std::remove_cvref_t<decltype(requested_command)>;

                    if constexpr (std::same_as<state_type, awaiting_roll> &&
                                  std::same_as<command_type, turn_action>) {
                        if (requested_command == turn_action::roll_completed) {
                            return roll_completed(state);
                        }
                    } else if constexpr (std::same_as<state_type, choosing_dice> &&
                                         std::same_as<command_type, turn_action>) {
                        if (requested_command == turn_action::reroll) {
                            return reroll(state);
                        }
                        if (requested_command == turn_action::score) {
                            return score(state);
                        }
                    } else if constexpr (std::same_as<state_type, choosing_dice> &&
                                         std::same_as<command_type, toggle_hold>) {
                        auto next = with_toggled_hold(state, requested_command.index);
                        if (!next) {
                            return std::unexpected{next.error()};
                        }
                        return turn_state{*next};
                    } else if constexpr (std::same_as<state_type, must_score> &&
                                         std::same_as<command_type, turn_action>) {
                        if (requested_command == turn_action::score) {
                            return score(state);
                        }
                    }

                    return std::unexpected{transition_error{
                        .code = transition_error_code::invalid_action,
                    }};
                },
                requested
            );
        },
        current
    );
}

} // namespace seamwork::studies::yahtzee_turn

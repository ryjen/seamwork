#pragma once

#include <array>
#include <cstddef>
#include <expected>
#include <span>
#include <stdexcept>
#include <variant>

#include <seamwork/studies/yahtzee_expected.hpp>
#include <seamwork/studies/yahtzee_randomness.hpp>
#include <seamwork/studies/yahtzee_turn_state.hpp>

namespace seamwork::examples::yahtzee_turn {

namespace randomness = seamwork::studies::yahtzee;
namespace scoring = seamwork::studies::yahtzee_expected;
namespace turn = seamwork::studies::yahtzee_turn;

using script_error = std::variant<
    turn::transition_error,
    scoring::category_error,
    scoring::hand_error>;

struct scripted_result {
    std::array<unsigned, randomness::dice_per_hand> faces;
    unsigned score{};
    turn::completed_roll_count rolls_taken;
};

class sequence_source
{
public:
    explicit constexpr sequence_source(
        std::array<randomness::face, 8> values
    ) noexcept
        : values_(values)
    {
    }

    [[nodiscard]] constexpr auto operator()() -> randomness::face
    {
        if (next_ >= values_.size()) {
            throw std::logic_error{"scripted face source exhausted"};
        }

        return values_[next_++];
    }

private:
    std::array<randomness::face, 8> values_;
    std::size_t next_{};
};

[[nodiscard]] constexpr auto to_scoring_values(const randomness::hand& hand)
    -> std::array<int, randomness::dice_per_hand>
{
    std::array<int, randomness::dice_per_hand> values{};

    for (std::size_t index = 0; index < values.size(); ++index) {
        values[index] = static_cast<int>(randomness::value(hand[index]));
    }

    return values;
}

[[nodiscard]] inline auto run_scripted_turn()
    -> std::expected<scripted_result, script_error>
{
    sequence_source source{{
        randomness::face::one,
        randomness::face::two,
        randomness::face::three,
        randomness::face::four,
        randomness::face::five,
        randomness::face::six,
        randomness::face::six,
        randomness::face::six,
    }};

    auto dice = randomness::initial_roll(source);
    auto state = turn::initial_turn();

    auto transition = turn::apply(state, turn::turn_action::roll_completed);
    if (!transition) {
        return std::unexpected{script_error{transition.error()}};
    }
    state = *transition;

    // Turn state is the single authority for hold decisions. Randomness
    // borrows the fixed-size selection only for the reroll operation.
    constexpr std::array hold_indices{std::size_t{1}, std::size_t{4}};
    for (const auto index : hold_indices) {
        transition = turn::apply(state, turn::toggle_hold{index});
        if (!transition) {
            return std::unexpected{script_error{transition.error()}};
        }
        state = *transition;
    }

    transition = turn::apply(state, turn::turn_action::reroll);
    if (!transition) {
        return std::unexpected{script_error{transition.error()}};
    }
    state = *transition;

    const auto* awaiting = std::get_if<turn::awaiting_roll>(&state);
    if (awaiting == nullptr) {
        return std::unexpected{script_error{turn::transition_error{
            .code = turn::transition_error_code::invalid_action,
        }}};
    }

    dice = randomness::reroll(
        dice,
        randomness::hold_view{awaiting->held()},
        source
    );

    transition = turn::apply(state, turn::turn_action::roll_completed);
    if (!transition) {
        return std::unexpected{script_error{transition.error()}};
    }
    state = *transition;

    const auto category = scoring::parse_category("sixes");
    if (!category) {
        return std::unexpected{script_error{category.error()}};
    }

    const auto raw_values = to_scoring_values(dice);
    const auto scoring_hand = scoring::make_hand(std::span<const int>{raw_values});
    if (!scoring_hand) {
        return std::unexpected{script_error{scoring_hand.error()}};
    }

    const auto final_score = scoring::score(*category, *scoring_hand);

    transition = turn::apply(state, turn::turn_action::score);
    if (!transition) {
        return std::unexpected{script_error{transition.error()}};
    }

    const auto* completed = std::get_if<turn::completed>(&*transition);
    if (completed == nullptr) {
        return std::unexpected{script_error{turn::transition_error{
            .code = turn::transition_error_code::invalid_action,
        }}};
    }

    std::array<unsigned, randomness::dice_per_hand> final_faces{};
    for (std::size_t index = 0; index < final_faces.size(); ++index) {
        final_faces[index] = randomness::value(dice[index]);
    }

    return scripted_result{
        .faces = final_faces,
        .score = final_score,
        .rolls_taken = completed->rolls_taken(),
    };
}

} // namespace seamwork::examples::yahtzee_turn

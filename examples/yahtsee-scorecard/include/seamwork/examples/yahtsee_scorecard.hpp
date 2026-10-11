#pragma once

#include <array>
#include <cstddef>
#include <expected>
#include <span>
#include <stdexcept>
#include <string_view>
#include <variant>

#include <seamwork/studies/yahtsee_expected.hpp>
#include <seamwork/studies/yahtsee_randomness.hpp>
#include <seamwork/studies/yahtsee_turn_state.hpp>

namespace seamwork::examples::yahtsee_scorecard {

namespace randomness = seamwork::studies::yahtsee;
namespace scoring = seamwork::studies::yahtsee_expected;
namespace turn = seamwork::studies::yahtsee_turn;

using script_error = std::variant<
    turn::transition_error,
    scoring::category_error,
    scoring::hand_error>;

struct scorecard_result {
    unsigned yahtsee_score{};
    unsigned chance_score{};
    unsigned total{};
};

template <std::size_t N>
class sequence_source
{
public:
    explicit constexpr sequence_source(std::array<randomness::face, N> values) noexcept
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
    std::array<randomness::face, N> values_;
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

[[nodiscard]] inline auto apply_command(turn::turn_state& state, const turn::command& command)
    -> std::expected<void, script_error>
{
    auto transition = turn::apply(state, command);
    if (!transition) {
        return std::unexpected{script_error{transition.error()}};
    }

    state = *transition;
    return {};
}

[[nodiscard]] inline auto score_hand(std::string_view category_name, const randomness::hand& dice)
    -> std::expected<unsigned, script_error>
{
    const auto category = scoring::parse_category(category_name);
    if (!category) {
        return std::unexpected{script_error{category.error()}};
    }

    // #46 deliberately keeps this explicit validation boundary. The second
    // consumer repeats it so promotion decisions are based on real evidence.
    const auto raw_values = to_scoring_values(dice);
    const auto scoring_hand = scoring::make_hand(std::span<const int>{raw_values});
    if (!scoring_hand) {
        return std::unexpected{script_error{scoring_hand.error()}};
    }

    return scoring::score(*category, *scoring_hand);
}

[[nodiscard]] inline auto play_yahtsee_turn()
    -> std::expected<unsigned, script_error>
{
    sequence_source source{std::array{
        randomness::face::six,
        randomness::face::six,
        randomness::face::two,
        randomness::face::three,
        randomness::face::four,
        randomness::face::six,
        randomness::face::six,
        randomness::face::six,
    }};

    auto dice = randomness::initial_roll(source);
    auto state = turn::initial_turn();

    if (auto result = apply_command(state, turn::turn_action::roll_completed); !result) {
        return std::unexpected{result.error()};
    }

    for (const auto index : std::array{std::size_t{0}, std::size_t{1}}) {
        if (auto result = apply_command(state, turn::toggle_hold{index}); !result) {
            return std::unexpected{result.error()};
        }
    }

    if (auto result = apply_command(state, turn::turn_action::reroll); !result) {
        return std::unexpected{result.error()};
    }

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

    if (auto result = apply_command(state, turn::turn_action::roll_completed); !result) {
        return std::unexpected{result.error()};
    }

    const auto points = score_hand("yahtsee", dice);
    if (!points) {
        return std::unexpected{points.error()};
    }

    if (auto result = apply_command(state, turn::turn_action::score); !result) {
        return std::unexpected{result.error()};
    }

    return *points;
}

[[nodiscard]] inline auto play_chance_turn()
    -> std::expected<unsigned, script_error>
{
    sequence_source source{std::array{
        randomness::face::one,
        randomness::face::two,
        randomness::face::three,
        randomness::face::four,
        randomness::face::five,
    }};

    const auto dice = randomness::initial_roll(source);
    auto state = turn::initial_turn();

    if (auto result = apply_command(state, turn::turn_action::roll_completed); !result) {
        return std::unexpected{result.error()};
    }

    const auto points = score_hand("chance", dice);
    if (!points) {
        return std::unexpected{points.error()};
    }

    if (auto result = apply_command(state, turn::turn_action::score); !result) {
        return std::unexpected{result.error()};
    }

    return *points;
}

[[nodiscard]] inline auto run_scripted_scorecard()
    -> std::expected<scorecard_result, script_error>
{
    const auto yahtsee = play_yahtsee_turn();
    if (!yahtsee) {
        return std::unexpected{yahtsee.error()};
    }

    const auto chance = play_chance_turn();
    if (!chance) {
        return std::unexpected{chance.error()};
    }

    return scorecard_result{
        .yahtsee_score = *yahtsee,
        .chance_score = *chance,
        .total = *yahtsee + *chance,
    };
}

} // namespace seamwork::examples::yahtsee_scorecard

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>

#include <seamwork/studies/yahtsee_turn_state.hpp>

namespace sw = seamwork::studies::yahtsee_turn;

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

auto step(const sw::turn_state& state, const sw::command& command) -> sw::turn_state
{
    const auto result = sw::apply(state, command);
    if (!result) {
        throw std::runtime_error{"unexpected invalid transition"};
    }
    return *result;
}

void test_initial_roll_enters_choice_state()
{
    auto state = sw::initial_turn();
    require(std::holds_alternative<sw::awaiting_roll>(state), "turn should begin awaiting a roll");

    state = step(state, sw::turn_action::roll_completed);
    require(std::holds_alternative<sw::choosing_dice>(state), "first roll should allow a choice");
    require(std::get<sw::choosing_dice>(state).rolls_taken() == sw::choice_roll_count::one, "first roll should be counted once");
}

void test_held_dice_survive_reroll_transition()
{
    auto state = step(sw::initial_turn(), sw::turn_action::roll_completed);
    state = step(state, sw::toggle_hold{2});

    const auto& choosing = std::get<sw::choosing_dice>(state);
    require(choosing.held()[2], "selected die should be held");

    state = step(state, sw::turn_action::reroll);
    const auto& awaiting = std::get<sw::awaiting_roll>(state);
    require(awaiting.rolls_taken() == sw::prior_roll_count::one, "reroll should preserve the completed-roll count");
    require(awaiting.held()[2], "reroll should preserve held dice");

    state = step(state, sw::turn_action::roll_completed);
    const auto& next_choice = std::get<sw::choosing_dice>(state);
    require(next_choice.rolls_taken() == sw::choice_roll_count::two, "second completed roll should increment the count");
    require(next_choice.held()[2], "held die should survive the completed reroll");
}

void test_third_roll_forces_scoring()
{
    auto state = step(sw::initial_turn(), sw::turn_action::roll_completed);
    state = step(state, sw::turn_action::reroll);
    state = step(state, sw::turn_action::roll_completed);
    state = step(state, sw::turn_action::reroll);
    state = step(state, sw::turn_action::roll_completed);

    require(std::holds_alternative<sw::must_score>(state), "third roll should force scoring");

    const auto invalid_reroll = sw::apply(state, sw::turn_action::reroll);
    require(!invalid_reroll, "reroll should be rejected after the third roll");
    require(
        invalid_reroll.error().code == sw::transition_error_code::invalid_action,
        "forced-score reroll should report an invalid action"
    );

    state = step(state, sw::turn_action::score);
    require(std::holds_alternative<sw::completed>(state), "scoring should complete the turn");
    require(std::get<sw::completed>(state).rolls_taken() == sw::completed_roll_count::three, "completed state should retain roll count");
}

void test_scoring_can_end_turn_early()
{
    auto state = step(sw::initial_turn(), sw::turn_action::roll_completed);
    state = step(state, sw::turn_action::score);

    require(std::holds_alternative<sw::completed>(state), "player may score after first roll");
    require(std::get<sw::completed>(state).rolls_taken() == sw::completed_roll_count::one, "early score should retain actual roll count");
}

void test_invalid_commands_are_explicit()
{
    const auto state = sw::initial_turn();

    const auto reroll = sw::apply(state, sw::turn_action::reroll);
    require(!reroll, "reroll before any roll should fail");
    require(reroll.error().code == sw::transition_error_code::invalid_action, "invalid transition should be typed");

    const auto hold = sw::apply(state, sw::toggle_hold{0});
    require(!hold, "holding before a roll should fail");
    require(hold.error().code == sw::transition_error_code::invalid_action, "invalid hold phase should be typed");
}

void test_invalid_die_index_is_distinct()
{
    const auto choosing = step(sw::initial_turn(), sw::turn_action::roll_completed);
    const auto result = sw::apply(choosing, sw::toggle_hold{sw::dice_per_hand});

    require(!result, "out-of-range hold should fail");
    require(
        result.error().code == sw::transition_error_code::die_out_of_range,
        "out-of-range hold should have a distinct error"
    );
    require(result.error().index == sw::dice_per_hand, "error should retain rejected index");
}

void test_completed_turn_rejects_more_commands()
{
    auto state = step(sw::initial_turn(), sw::turn_action::roll_completed);
    state = step(state, sw::turn_action::score);

    const auto result = sw::apply(state, sw::turn_action::roll_completed);
    require(!result, "completed turn should reject further commands");
    require(result.error().code == sw::transition_error_code::invalid_action, "completed transition should be typed");
}

} // namespace

int main()
{
    try {
        test_initial_roll_enters_choice_state();
        test_held_dice_survive_reroll_transition();
        test_third_roll_forces_scoring();
        test_scoring_can_end_turn_early();
        test_invalid_commands_are_explicit();
        test_invalid_die_index_is_distinct();
        test_completed_turn_rejects_more_commands();
    } catch (const std::exception& error) {
        std::cerr << "yahtsee-turn-state study failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#include <cstdlib>
#include <iostream>
#include <variant>

#include <seamwork/studies/yahtzee_turn_state.hpp>

namespace sw = seamwork::studies::yahtzee_turn;

auto apply_or_exit(const sw::turn_state& state, const sw::command& command) -> sw::turn_state
{
    const auto next = sw::apply(state, command);
    if (!next) {
        std::cerr << "invalid turn command\n";
        std::exit(2);
    }
    return *next;
}

int main()
{
    auto turn = sw::initial_turn();

    turn = apply_or_exit(turn, sw::turn_action::roll_completed);
    turn = apply_or_exit(turn, sw::toggle_hold{1});
    turn = apply_or_exit(turn, sw::turn_action::reroll);
    turn = apply_or_exit(turn, sw::turn_action::roll_completed);
    turn = apply_or_exit(turn, sw::turn_action::score);

    const auto& completed = std::get<sw::completed>(turn);
    std::cout << "turn completed after " << sw::value(completed.rolls_taken()) << " rolls\n";
    return 0;
}

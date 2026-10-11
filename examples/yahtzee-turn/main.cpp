#include <cstdlib>
#include <iostream>

#include <seamwork/examples/yahtzee_turn.hpp>

namespace example = seamwork::examples::yahtzee_turn;
namespace turn = seamwork::studies::yahtzee_turn;

int main()
{
    const auto result = example::run_scripted_turn();
    if (!result) {
        std::cerr << "scripted turn failed at integration boundary\n";
        return EXIT_FAILURE;
    }

    std::cout << "final faces:";
    for (const auto face : result->faces) {
        std::cout << ' ' << face;
    }

    std::cout << "\nscore: " << result->score
              << "\nrolls: " << turn::value(result->rolls_taken)
              << '\n';

    return EXIT_SUCCESS;
}

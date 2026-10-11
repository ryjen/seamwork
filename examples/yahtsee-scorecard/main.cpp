#include <cstdlib>
#include <iostream>

#include <seamwork/examples/yahtsee_scorecard.hpp>

namespace example = seamwork::examples::yahtsee_scorecard;

int main()
{
    const auto result = example::run_scripted_scorecard();
    if (!result) {
        std::cerr << "scripted scorecard failed at integration boundary\n";
        return EXIT_FAILURE;
    }

    std::cout << "Yahtsee: " << result->yahtsee_score
              << "\nChance: " << result->chance_score
              << "\nTotal: " << result->total
              << '\n';

    return EXIT_SUCCESS;
}

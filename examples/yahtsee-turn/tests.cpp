#include <array>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <seamwork/examples/yahtsee_turn.hpp>

namespace example = seamwork::examples::yahtsee_turn;
namespace turn = seamwork::studies::yahtsee_turn;

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

void test_composed_turn()
{
    const auto result = example::run_scripted_turn();
    require(result.has_value(), "composed scripted turn should succeed");

    const std::array<unsigned, 5> expected_faces{6, 2, 6, 6, 5};
    require(result->faces == expected_faces, "reroll should preserve held dice and replace unheld dice");
    require(result->score == 18U, "sixes score should be eighteen");
    require(
        result->rolls_taken == turn::completed_roll_count::two,
        "scripted turn should score after the second roll"
    );
}

} // namespace

int main()
{
    try {
        test_composed_turn();
    } catch (const std::exception& error) {
        std::cerr << "yahtsee composed example failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <seamwork/examples/yahtsee_scorecard.hpp>

namespace example = seamwork::examples::yahtsee_scorecard;

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

void test_two_turn_scorecard()
{
    const auto result = example::run_scripted_scorecard();
    require(result.has_value(), "scripted scorecard should succeed");
    require(result->yahtsee_score == 50U, "five sixes should score Yahtsee");
    require(result->chance_score == 15U, "1+2+3+4+5 should score fifteen for chance");
    require(result->total == 65U, "scorecard should accumulate both categories");
}

} // namespace

int main()
{
    try {
        test_two_turn_scorecard();
    } catch (const std::exception& error) {
        std::cerr << "yahtsee scorecard example failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

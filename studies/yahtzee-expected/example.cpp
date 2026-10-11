#include <array>
#include <iostream>
#include <span>
#include <string_view>

#include <seamwork/studies/yahtzee_expected.hpp>

namespace sw = seamwork::studies::yahtzee_expected;

int main()
{
    constexpr std::string_view requested_category = "sixes";
    const std::array raw_dice{6, 6, 2, 6, 1};

    const auto category = sw::parse_category(requested_category);
    if (!category) {
        std::cerr << "unknown scoring category: " << category.error().input << '\n';
        return 2;
    }

    const auto hand = sw::make_hand(std::span<const int>{raw_dice});
    if (!hand) {
        const auto& error = hand.error();
        if (error.code == sw::hand_error_code::wrong_count) {
            std::cerr << "expected five dice, got " << error.count << '\n';
        } else {
            std::cerr << "invalid die value " << error.value << " at index " << error.index << '\n';
        }
        return 2;
    }

    std::cout << "score: " << sw::score(*category, *hand) << '\n';
    return 0;
}

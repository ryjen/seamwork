#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include <seamwork/yahtsee/domain.hpp>

namespace seamwork::studies::yahtsee_expected {

inline constexpr std::size_t dice_per_hand = seamwork::yahtsee::dice_per_hand;
inline constexpr int minimum_face = seamwork::yahtsee::minimum_face;
inline constexpr int maximum_face = seamwork::yahtsee::maximum_face;

using hand = seamwork::yahtsee::hand;
using hand_error = seamwork::yahtsee::hand_error;
using hand_error_code = seamwork::yahtsee::hand_error_code;

enum class category : std::uint8_t {
    ones = 1,
    twos,
    threes,
    fours,
    fives,
    sixes,
    chance,
    yahtsee,
};

struct category_error {
    std::string input;
};

[[nodiscard]] inline auto parse_category(std::string_view input)
    -> std::expected<category, category_error>
{
    using pair = std::pair<std::string_view, category>;
    constexpr std::array categories{
        pair{"ones", category::ones},
        pair{"twos", category::twos},
        pair{"threes", category::threes},
        pair{"fours", category::fours},
        pair{"fives", category::fives},
        pair{"sixes", category::sixes},
        pair{"chance", category::chance},
        pair{"yahtsee", category::yahtsee},
    };

    for (const auto& [name, value] : categories) {
        if (input == name) {
            return value;
        }
    }

    return std::unexpected{category_error{std::string{input}}};
}

[[nodiscard]] inline auto make_hand(std::span<const int> raw)
    -> std::expected<hand, hand_error>
{
    return seamwork::yahtsee::make_hand(raw);
}

[[nodiscard]] constexpr auto score(category selected, const hand& dice) noexcept -> unsigned
{
    const auto& values = dice.values();

    if (selected == category::chance) {
        unsigned total = 0;
        for (const auto side : values) {
            total += seamwork::yahtsee::value(side);
        }
        return total;
    }

    if (selected == category::yahtsee) {
        const auto first = values.front();
        for (const auto side : values) {
            if (side != first) {
                return 0U;
            }
        }
        return 50U;
    }

    const auto wanted = static_cast<unsigned>(selected);
    unsigned total = 0;
    for (const auto side : values) {
        if (seamwork::yahtsee::value(side) == wanted) {
            total += seamwork::yahtsee::value(side);
        }
    }
    return total;
}

} // namespace seamwork::studies::yahtsee_expected

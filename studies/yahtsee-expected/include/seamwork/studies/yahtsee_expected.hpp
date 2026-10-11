#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <numeric>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace seamwork::studies::yahtsee_expected {

inline constexpr std::size_t dice_per_hand = 5;
inline constexpr int minimum_face = 1;
inline constexpr int maximum_face = 6;

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

enum class hand_error_code : std::uint8_t {
    wrong_count,
    face_out_of_range,
};

struct hand_error {
    hand_error_code code;
    std::size_t index{};
    int value{};
    std::size_t count{};
};

class hand
{
public:
    using values_type = std::array<std::uint8_t, dice_per_hand>;

    [[nodiscard]] constexpr auto values() const noexcept -> const values_type&
    {
        return values_;
    }

private:
    friend auto make_hand(std::span<const int>) -> std::expected<hand, hand_error>;

    constexpr explicit hand(values_type values) noexcept
        : values_(values)
    {
    }

    values_type values_;
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
    if (raw.size() != dice_per_hand) {
        return std::unexpected{hand_error{
            .code = hand_error_code::wrong_count,
            .count = raw.size(),
        }};
    }

    hand::values_type values{};
    for (std::size_t index = 0; index < raw.size(); ++index) {
        if (raw[index] < minimum_face || raw[index] > maximum_face) {
            return std::unexpected{hand_error{
                .code = hand_error_code::face_out_of_range,
                .index = index,
                .value = raw[index],
                .count = raw.size(),
            }};
        }

        values[index] = static_cast<std::uint8_t>(raw[index]);
    }

    return hand{values};
}

[[nodiscard]] constexpr auto score(category selected, const hand& dice) noexcept -> unsigned
{
    const auto& values = dice.values();

    if (selected == category::chance) {
        return std::accumulate(values.begin(), values.end(), 0U);
    }

    if (selected == category::yahtsee) {
        const auto first = values.front();
        for (const auto value : values) {
            if (value != first) {
                return 0U;
            }
        }
        return 50U;
    }

    const auto wanted = static_cast<unsigned>(selected);
    unsigned total = 0;
    for (const auto value : values) {
        if (value == wanted) {
            total += value;
        }
    }
    return total;
}

} // namespace seamwork::studies::yahtsee_expected

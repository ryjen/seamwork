#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <stdexcept>
#include <utility>

namespace seamwork::yahtsee {

inline constexpr std::size_t dice_per_hand = 5;
inline constexpr int minimum_face = 1;
inline constexpr int maximum_face = 6;

enum class face : std::uint8_t {
    one = 1,
    two,
    three,
    four,
    five,
    six,
};

[[nodiscard]] constexpr auto value(face side) noexcept -> unsigned
{
    return static_cast<unsigned>(std::to_underlying(side));
}

using face_values = std::array<face, dice_per_hand>;

class hand
{
public:
    constexpr explicit hand(face_values values)
        : values_(values)
    {
        for (const auto side : values_) {
            if (value(side) < static_cast<unsigned>(minimum_face)
                || value(side) > static_cast<unsigned>(maximum_face)) {
                throw std::invalid_argument{"Yahtsee hand contains an invalid face"};
            }
        }
    }

    [[nodiscard]] constexpr auto values() const noexcept -> const face_values&
    {
        return values_;
    }

    [[nodiscard]] constexpr auto operator[](std::size_t index) const -> face
    {
        return values_.at(index);
    }

private:
    face_values values_;
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

[[nodiscard]] inline auto make_hand(std::span<const int> raw)
    -> std::expected<hand, hand_error>
{
    if (raw.size() != dice_per_hand) {
        return std::unexpected{hand_error{
            .code = hand_error_code::wrong_count,
            .count = raw.size(),
        }};
    }

    face_values values{};
    for (std::size_t index = 0; index < raw.size(); ++index) {
        if (raw[index] < minimum_face || raw[index] > maximum_face) {
            return std::unexpected{hand_error{
                .code = hand_error_code::face_out_of_range,
                .index = index,
                .value = raw[index],
                .count = raw.size(),
            }};
        }

        values[index] = static_cast<face>(raw[index]);
    }

    return hand{values};
}

} // namespace seamwork::yahtsee

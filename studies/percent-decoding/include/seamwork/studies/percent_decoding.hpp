#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace seamwork::studies::percent_decoding {

enum class plus_policy : std::uint8_t {
    literal,
    space,
};

enum class decode_error_code : std::uint8_t {
    truncated_escape,
    invalid_hex_digit,
};

struct decode_error {
    decode_error_code code;
    std::size_t index{};
    char value{};
};

namespace detail {

[[nodiscard]] constexpr auto hex_value(char value) noexcept -> int
{
    if (value >= '0' && value <= '9') {
        return value - '0';
    }
    if (value >= 'A' && value <= 'F') {
        return value - 'A' + 10;
    }
    if (value >= 'a' && value <= 'f') {
        return value - 'a' + 10;
    }
    return -1;
}

} // namespace detail

[[nodiscard]] inline auto decode_percent(
    std::string_view input,
    plus_policy plus = plus_policy::literal
) -> std::expected<std::string, decode_error>
{
    std::string output;
    output.reserve(input.size());

    std::size_t index = 0;
    while (index < input.size()) {
        const auto current = input[index];

        if (current == '%') {
            const auto remaining = input.size() - index;
            if (remaining < 3) {
                return std::unexpected{decode_error{
                    .code = decode_error_code::truncated_escape,
                    .index = index,
                    .value = current,
                }};
            }

            const auto high = detail::hex_value(input[index + 1]);
            if (high < 0) {
                return std::unexpected{decode_error{
                    .code = decode_error_code::invalid_hex_digit,
                    .index = index + 1,
                    .value = input[index + 1],
                }};
            }

            const auto low = detail::hex_value(input[index + 2]);
            if (low < 0) {
                return std::unexpected{decode_error{
                    .code = decode_error_code::invalid_hex_digit,
                    .index = index + 2,
                    .value = input[index + 2],
                }};
            }

            const auto decoded = static_cast<unsigned>((high << 4) | low);
            output.push_back(static_cast<char>(decoded));
            index += 3;
            continue;
        }

        if (current == '+' && plus == plus_policy::space) {
            output.push_back(' ');
        } else {
            output.push_back(current);
        }

        ++index;
    }

    return output;
}

namespace alternatives {

[[nodiscard]] inline auto decode_or_throw(
    std::string_view input,
    plus_policy plus = plus_policy::literal
) -> std::string
{
    const auto decoded = decode_percent(input, plus);
    if (!decoded) {
        throw decode_error{decoded.error()};
    }
    return *decoded;
}

[[nodiscard]] inline auto decode_lenient(std::string_view input) -> std::string
{
    std::string output;
    output.reserve(input.size());

    std::size_t index = 0;
    while (index < input.size()) {
        if (input[index] == '%' && input.size() - index >= 3) {
            const auto high = detail::hex_value(input[index + 1]);
            const auto low = detail::hex_value(input[index + 2]);
            if (high >= 0 && low >= 0) {
                output.push_back(static_cast<char>((high << 4) | low));
                index += 3;
                continue;
            }
        }

        output.push_back(input[index]);
        ++index;
    }

    return output;
}

} // namespace alternatives

} // namespace seamwork::studies::percent_decoding

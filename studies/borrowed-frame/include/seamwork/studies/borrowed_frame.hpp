#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <vector>

namespace seamwork::studies::borrowed_frame {

inline constexpr std::size_t header_size = 2;
inline constexpr std::size_t default_max_payload = 4096;

enum class parse_error_code : std::uint8_t {
    null_input,
    truncated_header,
    payload_too_large,
    truncated_payload,
};

struct parse_error {
    parse_error_code code;
    std::size_t declared_length{};
    std::size_t available_bytes{};
    std::size_t max_payload{};
};

struct frame_view {
    std::span<const std::byte> payload;
    std::size_t consumed{};
};

struct owned_frame {
    std::vector<std::byte> payload;
    std::size_t consumed{};
};

[[nodiscard]] constexpr auto declared_payload_length(
    std::span<const std::byte, header_size> header
) noexcept -> std::size_t
{
    const auto high = std::to_integer<unsigned>(header[0]);
    const auto low = std::to_integer<unsigned>(header[1]);
    return static_cast<std::size_t>((high << 8U) | low);
}

[[nodiscard]] constexpr auto parse(
    std::span<const std::byte> input,
    std::size_t max_payload = default_max_payload
) -> std::expected<frame_view, parse_error>
{
    if (input.size() < header_size) {
        return std::unexpected{parse_error{
            .code = parse_error_code::truncated_header,
            .available_bytes = input.size(),
            .max_payload = max_payload,
        }};
    }

    const auto header = std::span<const std::byte, header_size>{input.first<header_size>()};
    const auto payload_length = declared_payload_length(header);

    if (payload_length > max_payload) {
        return std::unexpected{parse_error{
            .code = parse_error_code::payload_too_large,
            .declared_length = payload_length,
            .available_bytes = input.size() - header_size,
            .max_payload = max_payload,
        }};
    }

    const auto available_payload = input.size() - header_size;
    if (payload_length > available_payload) {
        return std::unexpected{parse_error{
            .code = parse_error_code::truncated_payload,
            .declared_length = payload_length,
            .available_bytes = available_payload,
            .max_payload = max_payload,
        }};
    }

    return frame_view{
        .payload = input.subspan(header_size, payload_length),
        .consumed = header_size + payload_length,
    };
}

[[nodiscard]] constexpr auto parse_raw(
    const std::byte* data,
    std::size_t size,
    std::size_t max_payload = default_max_payload
) -> std::expected<frame_view, parse_error>
{
    if (data == nullptr && size != 0) {
        return std::unexpected{parse_error{
            .code = parse_error_code::null_input,
            .available_bytes = size,
            .max_payload = max_payload,
        }};
    }

    return parse(std::span<const std::byte>{data, size}, max_payload);
}

[[nodiscard]] inline auto retain(frame_view frame) -> owned_frame
{
    return owned_frame{
        .payload = std::vector<std::byte>{frame.payload.begin(), frame.payload.end()},
        .consumed = frame.consumed,
    };
}

} // namespace seamwork::studies::borrowed_frame

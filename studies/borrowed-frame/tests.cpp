#include <array>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <seamwork/studies/borrowed_frame.hpp>

namespace sw = seamwork::studies::borrowed_frame;

namespace {

template <class Range>
concept parseable_range = requires(Range&& range) {
    sw::parse(std::forward<Range>(range));
};

static_assert(parseable_range<std::array<std::byte, 2>&>);
static_assert(parseable_range<std::vector<std::byte>&>);
static_assert(parseable_range<std::span<const std::byte>>);
static_assert(!parseable_range<std::array<std::byte, 2>>);
static_assert(!parseable_range<std::vector<std::byte>>);

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

constexpr auto b(unsigned value) -> std::byte
{
    return static_cast<std::byte>(value);
}

void test_borrowed_parse_with_trailing_bytes()
{
    std::array bytes{
        b(0x00), b(0x03),
        b('c'), b('a'), b('t'),
        b(0xFF), b(0xEE),
    };

    const auto result = sw::parse(bytes);
    require(result.has_value(), "valid frame should parse");
    require(result->payload.size() == 3, "payload length should come from header");
    require(result->consumed == 5, "parser should report bytes consumed");
    require(result->payload.data() == bytes.data() + sw::header_size, "payload should borrow caller storage");
    require(result->payload[0] == b('c'), "borrowed payload should expose frame bytes");
    require(result->payload[2] == b('t'), "borrowed payload should preserve final byte");
}

void test_truncated_header()
{
    const std::array<std::byte, 0> empty{};
    const auto empty_result = sw::parse(empty);
    require(!empty_result, "empty input should fail");
    require(
        empty_result.error().code == sw::parse_error_code::truncated_header,
        "empty input should report truncated header"
    );

    const std::array one{b(0x00)};
    const auto one_result = sw::parse(one);
    require(!one_result, "one-byte input should fail");
    require(one_result.error().available_bytes == 1, "header error should retain available size");
}

void test_zero_length_payload()
{
    const std::array bytes{b(0x00), b(0x00), b(0xAA)};
    const auto result = sw::parse(bytes);
    require(result.has_value(), "zero-length frame should be valid");
    require(result->payload.empty(), "zero-length frame should have empty payload");
    require(result->consumed == sw::header_size, "zero-length frame should consume only header");
}

void test_truncated_payload()
{
    const std::array bytes{b(0x00), b(0x04), b(0x10), b(0x20)};
    const auto result = sw::parse(bytes);
    require(!result, "truncated payload should fail");
    require(
        result.error().code == sw::parse_error_code::truncated_payload,
        "truncated payload should have typed error"
    );
    require(result.error().declared_length == 4, "error should retain declared length");
    require(result.error().available_bytes == 2, "error should retain available payload bytes");
}

void test_payload_limit()
{
    const std::array bytes{
        b(0x00), b(0x05),
        b(1), b(2), b(3), b(4), b(5),
    };

    const auto rejected = sw::parse(bytes, 4);
    require(!rejected, "declared payload beyond configured limit should fail");
    require(
        rejected.error().code == sw::parse_error_code::payload_too_large,
        "over-limit payload should have typed error"
    );
    require(rejected.error().max_payload == 4, "error should retain configured limit");

    const auto accepted = sw::parse(bytes, 5);
    require(accepted.has_value(), "payload at configured limit should parse");
}

void test_raw_pointer_baseline_rejects_mismatch()
{
    const auto result = sw::parse_raw(nullptr, 3);
    require(!result, "null pointer with nonzero length should fail");
    require(
        result.error().code == sw::parse_error_code::null_input,
        "raw pointer mismatch should need an extra error case"
    );
}

void test_explicit_retention_copies_payload()
{
    std::array bytes{b(0x00), b(0x02), b(0xAA), b(0xBB)};
    const auto view = sw::parse(bytes);
    require(view.has_value(), "valid frame should parse");

    const auto owned = sw::retain(*view);
    bytes[2] = b(0x11);

    require(view->payload[0] == b(0x11), "borrowed view should observe caller storage");
    require(owned.payload[0] == b(0xAA), "retained frame should own an independent copy");
    require(owned.consumed == view->consumed, "retention should preserve consumed count");
}

} // namespace

int main()
{
    try {
        test_borrowed_parse_with_trailing_bytes();
        test_truncated_header();
        test_zero_length_payload();
        test_truncated_payload();
        test_payload_limit();
        test_raw_pointer_baseline_rejects_mismatch();
        test_explicit_retention_copies_payload();
    } catch (const std::exception& error) {
        std::cerr << "borrowed-frame study failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

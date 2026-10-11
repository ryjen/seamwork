#include <array>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <seamwork/studies/native_size_boundary.hpp>

namespace sw = seamwork::studies::native_size_boundary;

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

void test_checked_count_boundaries()
{
    const auto zero = sw::checked_native_count(0);
    require(zero.has_value() && *zero == 0, "zero should fit native int count");

    const auto maximum = sw::checked_native_count(static_cast<std::size_t>(INT_MAX));
    require(
        maximum.has_value() && *maximum == INT_MAX,
        "INT_MAX should remain representable"
    );

    const auto too_large =
        sw::checked_native_count(static_cast<std::size_t>(INT_MAX) + std::size_t{1});
    require(!too_large, "INT_MAX + 1 should be rejected before narrowing");
    require(
        too_large.error().code == sw::size_error_code::not_representable,
        "oversized count should have typed conversion failure"
    );
}

void test_zero_length_skips_native_call()
{
    sw::fake_native_sink sink;
    const std::span<const std::byte> empty{};

    const auto result = sw::write_once(empty, sink);
    require(result.has_value() && *result == 0U, "zero-length write should complete as zero");
    require(sink.calls() == 0U, "zero-length write should not enter native API");
}

void test_valid_size_is_narrowed_only_after_validation()
{
    const std::array bytes{
        std::byte{0x01},
        std::byte{0x02},
        std::byte{0x03},
    };
    sw::fake_native_sink sink;

    const auto result = sw::write_once(bytes, sink);
    require(result.has_value() && *result == bytes.size(), "native full write should succeed");
    require(sink.calls() == 1U, "native API should be called once");
    require(sink.last_count() == 3, "validated count should reach native API exactly");
}

void test_negative_native_result_is_typed_failure()
{
    const std::array bytes{std::byte{0xAA}, std::byte{0xBB}};
    sw::fake_native_sink sink;
    sink.fail_next();

    const auto result = sw::write_once(bytes, sink);
    require(!result, "negative native return should fail");
    require(
        result.error().code == sw::write_error_code::native_failure,
        "negative native result should have typed error"
    );
    require(result.error().native_result == -1, "native error result should remain observable");
}

void test_partial_write_remains_observable()
{
    const std::array bytes{
        std::byte{0x10},
        std::byte{0x20},
        std::byte{0x30},
        std::byte{0x40},
    };
    sw::fake_native_sink sink;
    sink.return_next(2);

    const auto result = sw::write_once(bytes, sink);
    require(result.has_value(), "partial positive native write should remain a successful count");
    require(*result == 2U, "caller should see exact partial-write count");
    require(sink.calls() == 1U, "adapter should not hide a retry loop");
}

} // namespace

int main()
{
    try {
        test_checked_count_boundaries();
        test_zero_length_skips_native_call();
        test_valid_size_is_narrowed_only_after_validation();
        test_negative_native_result_is_typed_failure();
        test_partial_write_remains_observable();
    } catch (const std::exception& error) {
        std::cerr << "native-size-boundary study failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

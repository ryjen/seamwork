#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <seamwork/studies/percent_decoding.hpp>

namespace sw = seamwork::studies::percent_decoding;

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

void test_valid_mixed_case_hex()
{
    const auto decoded = sw::decode_percent("hello%20world%2fA%2Fb");
    require(decoded.has_value(), "valid percent escapes should decode");
    require(*decoded == "hello world/A/b", "mixed-case hex should decode identically");
}

void test_truncated_escape()
{
    const auto trailing = sw::decode_percent("abc%");
    require(!trailing, "trailing percent should fail");
    require(
        trailing.error().code == sw::decode_error_code::truncated_escape,
        "trailing percent should report truncated escape"
    );
    require(trailing.error().index == 3, "truncated error should identify percent index");

    const auto one_digit = sw::decode_percent("abc%A");
    require(!one_digit, "single hex digit should fail");
    require(
        one_digit.error().code == sw::decode_error_code::truncated_escape,
        "single hex digit should report truncated escape"
    );
}

void test_invalid_hex_digit()
{
    const auto high = sw::decode_percent("%GG");
    require(!high, "invalid high nibble should fail");
    require(
        high.error().code == sw::decode_error_code::invalid_hex_digit,
        "invalid high nibble should have typed error"
    );
    require(high.error().index == 1, "invalid high nibble index should be reported");
    require(high.error().value == 'G', "invalid high nibble value should be retained");

    const auto low = sw::decode_percent("%0Z");
    require(!low, "invalid low nibble should fail");
    require(low.error().index == 2, "invalid low nibble index should be reported");
    require(low.error().value == 'Z', "invalid low nibble value should be retained");
}

void test_decoded_nul_is_preserved()
{
    const auto decoded = sw::decode_percent("A%00B");
    require(decoded.has_value(), "NUL escape should decode");
    require(decoded->size() == 3, "decoded NUL should remain part of owned string");
    require((*decoded)[0] == 'A', "prefix should remain intact");
    require((*decoded)[1] == '\0', "decoded byte should be NUL");
    require((*decoded)[2] == 'B', "suffix should remain intact");
}

void test_high_byte_is_preserved()
{
    const auto decoded = sw::decode_percent("%FF");
    require(decoded.has_value(), "0xFF escape should decode");
    require(decoded->size() == 1, "high-byte escape should produce one byte");
    require(
        static_cast<unsigned char>((*decoded)[0]) == 0xFFU,
        "decoded high byte should preserve its object representation"
    );
}

void test_plus_policy_is_explicit()
{
    const auto literal = sw::decode_percent("a+b");
    require(literal.has_value() && *literal == "a+b", "plus should be literal by default");

    const auto form = sw::decode_percent("a+b", sw::plus_policy::space);
    require(form.has_value() && *form == "a b", "plus-to-space should require explicit policy");
}

void test_exception_alternative_preserves_error_information()
{
    bool caught = false;
    try {
        static_cast<void>(sw::alternatives::decode_or_throw("%Q0"));
    } catch (const sw::decode_exception& exception) {
        caught = true;
        require(
            exception.error().code == sw::decode_error_code::invalid_hex_digit,
            "throwing alternative should retain typed error"
        );
        require(exception.error().index == 1, "throwing alternative should retain error position");
    }

    require(caught, "throwing alternative should throw malformed-input error");
}

void test_lenient_alternative_passes_invalid_escape_through()
{
    require(
        sw::alternatives::decode_lenient("ok%20bad%GGend%") == "ok bad%GGend%",
        "lenient alternative should preserve malformed escapes literally"
    );
}

} // namespace

int main()
{
    try {
        test_valid_mixed_case_hex();
        test_truncated_escape();
        test_invalid_hex_digit();
        test_decoded_nul_is_preserved();
        test_high_byte_is_preserved();
        test_plus_policy_is_explicit();
        test_exception_alternative_preserves_error_information();
        test_lenient_alternative_passes_invalid_escape_through();
    } catch (const std::exception& error) {
        std::cerr << "percent-decoding study failed: " << error.what() << '\n';
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

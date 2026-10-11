#include <array>
#include <cstdlib>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include <seamwork/studies/yahtsee_expected.hpp>

namespace sw = seamwork::studies::yahtsee_expected;

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

void test_category_parsing()
{
    const auto valid = sw::parse_category("yahtsee");
    require(valid.has_value(), "known category should parse");
    require(*valid == sw::category::yahtsee, "parsed category should preserve meaning");

    const auto invalid = sw::parse_category("pair");
    require(!invalid.has_value(), "unknown category should fail explicitly");
    require(invalid.error().input == "pair", "parse error should retain rejected input");
}

void test_hand_validation()
{
    const std::array too_few{1, 2, 3, 4};
    const auto wrong_count = sw::make_hand(std::span<const int>{too_few});
    require(!wrong_count.has_value(), "wrong die count should fail");
    require(wrong_count.error().code == sw::hand_error_code::wrong_count, "wrong die count should have a typed error");
    require(wrong_count.error().count == 4, "wrong-count error should retain actual count");

    const std::array invalid_face{1, 2, 7, 4, 5};
    const auto out_of_range = sw::make_hand(std::span<const int>{invalid_face});
    require(!out_of_range.has_value(), "out-of-range face should fail");
    require(out_of_range.error().code == sw::hand_error_code::face_out_of_range, "invalid face should have a typed error");
    require(out_of_range.error().index == 2, "invalid-face error should retain index");
    require(out_of_range.error().value == 7, "invalid-face error should retain value");
}

void test_scoring_after_validation()
{
    const std::array raw{6, 6, 2, 6, 1};
    const auto hand = sw::make_hand(std::span<const int>{raw});
    require(hand.has_value(), "valid hand should construct");

    static_assert(std::is_same_v<decltype(sw::score(sw::category::sixes, *hand)), unsigned>);

    require(sw::score(sw::category::sixes, *hand) == 18U, "sixes should total matching dice");
    require(sw::score(sw::category::ones, *hand) == 1U, "ones should total matching dice");
    require(sw::score(sw::category::chance, *hand) == 21U, "chance should total all dice");
    require(sw::score(sw::category::yahtsee, *hand) == 0U, "mixed hand should not score yahtsee");
}

void test_yahtsee_score()
{
    const std::array raw{5, 5, 5, 5, 5};
    const auto hand = sw::make_hand(std::span<const int>{raw});
    require(hand.has_value(), "valid yahtsee hand should construct");
    require(sw::score(sw::category::yahtsee, *hand) == 50U, "yahtsee should score fifty");
}

} // namespace

int main()
{
    try {
        test_category_parsing();
        test_hand_validation();
        test_scoring_after_validation();
        test_yahtsee_score();
    } catch (const std::exception& error) {
        std::cerr << "yahtsee-expected study failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

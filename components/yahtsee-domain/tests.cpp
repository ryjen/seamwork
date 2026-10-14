#include <array>
#include <cstdlib>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

#include <seamwork/yahtsee/domain.hpp>

namespace domain = seamwork::yahtsee;

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

void test_typed_hand_preserves_faces()
{
    const domain::hand hand{{
        domain::face::one,
        domain::face::two,
        domain::face::three,
        domain::face::four,
        domain::face::five,
    }};

    require(hand[0] == domain::face::one, "first face should be preserved");
    require(hand[4] == domain::face::five, "last face should be preserved");
    require(domain::value(hand[2]) == 3U, "face value should expose domain ordinal");
}

void test_typed_hand_rejects_invalid_faces()
{
    for (const auto invalid : std::array{
             static_cast<domain::face>(0),
             static_cast<domain::face>(7),
             static_cast<domain::face>(255),
         }) {
        bool rejected = false;
        try {
            const domain::hand bad{{
                domain::face::one,
                domain::face::two,
                invalid,
                domain::face::four,
                domain::face::five,
            }};
            static_cast<void>(bad);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }

        require(rejected, "typed hand should reject invalid enum representations");
    }
}

void test_raw_hand_validation()
{
    const std::array valid{1, 2, 3, 4, 5};
    const auto hand = domain::make_hand(std::span<const int>{valid});
    require(hand.has_value(), "five in-range values should construct a hand");
    require((*hand)[3] == domain::face::four, "validated hand should preserve typed faces");

    const std::array too_few{1, 2, 3, 4};
    const auto wrong_count = domain::make_hand(std::span<const int>{too_few});
    require(!wrong_count, "wrong-size raw input should fail");
    require(
        wrong_count.error().code == domain::hand_error_code::wrong_count,
        "wrong-size input should have a typed error"
    );

    const std::array invalid{1, 2, 7, 4, 5};
    const auto out_of_range = domain::make_hand(std::span<const int>{invalid});
    require(!out_of_range, "out-of-range raw input should fail");
    require(
        out_of_range.error().code == domain::hand_error_code::face_out_of_range,
        "invalid face should have a typed error"
    );
    require(out_of_range.error().index == 2, "invalid face error should retain index");
    require(out_of_range.error().value == 7, "invalid face error should retain value");
}

} // namespace

int main()
{
    try {
        test_typed_hand_preserves_faces();
        test_typed_hand_rejects_invalid_faces();
        test_raw_hand_validation();
    } catch (const std::exception& error) {
        std::cerr << "Yahtsee domain component failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

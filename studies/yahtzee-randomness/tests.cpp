#include <array>
#include <cstdlib>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <seamwork/studies/yahtzee_randomness.hpp>

namespace sw = seamwork::studies::yahtzee;

namespace {

class sequence_source
{
public:
    explicit sequence_source(std::vector<sw::face> values)
        : values_(std::move(values))
    {
    }

    auto operator()() -> sw::face
    {
        if (next_ >= values_.size()) {
            throw std::out_of_range{"sequence source exhausted"};
        }
        ++calls_;
        return values_[next_++];
    }

    [[nodiscard]] auto calls() const noexcept -> std::size_t
    {
        return calls_;
    }

private:
    std::vector<sw::face> values_;
    std::size_t next_{};
    std::size_t calls_{};
};

class virtual_sequence_source final : public sw::alternatives::virtual_face_source
{
public:
    explicit virtual_sequence_source(std::vector<sw::face> values)
        : source_(std::move(values))
    {
    }

    auto next() -> sw::face override
    {
        return source_();
    }

private:
    sequence_source source_;
};

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

void test_semantic_callable_seam()
{
    sequence_source initial{{
        sw::face::one,
        sw::face::two,
        sw::face::three,
        sw::face::four,
        sw::face::five,
    }};

    auto hand = sw::initial_roll(initial);
    require(initial.calls() == sw::dice_per_hand, "initial roll should consume five faces");

    const std::array<bool, sw::dice_per_hand> held{false, true, false, false, true};
    sequence_source reroll{{
        sw::face::six,
        sw::face::five,
        sw::face::four,
    }};

    hand = sw::reroll(hand, sw::hold_view{held}, reroll);

    require(reroll.calls() == 3, "reroll should consume only unheld dice");
    require(hand[0] == sw::face::six, "first unheld die should reroll");
    require(hand[1] == sw::face::two, "held die should survive reroll");
    require(hand[2] == sw::face::five, "middle unheld die should reroll");
    require(hand[3] == sw::face::four, "last unheld die should reroll");
    require(hand[4] == sw::face::five, "held final die should survive reroll");
}

void test_hold_contract_is_fixed_extent_and_borrowed()
{
    static_assert(sw::hold_view::extent == sw::dice_per_hand);

    sequence_source source{{
        sw::face::six,
        sw::face::six,
        sw::face::six,
        sw::face::six,
        sw::face::six,
    }};
    const sw::hand hand{{
        sw::face::one,
        sw::face::two,
        sw::face::three,
        sw::face::four,
        sw::face::five,
    }};
    const std::array<bool, sw::dice_per_hand> held{true, true, true, true, true};

    const auto rerolled = sw::reroll(hand, sw::hold_view{held}, source);
    require(source.calls() == 0, "fully held hand should not consume the source");
    require(rerolled.values() == hand.values(), "borrowed holds should preserve all held values");
}

void test_uniform_source_produces_domain_faces()
{
    sw::uniform_face_source source{std::mt19937{42}};

    for (int sample = 0; sample < 100; ++sample) {
        const auto side = sw::value(source());
        require(side >= 1 && side <= sw::sides_per_die, "uniform source returned invalid face");
    }
}

void test_virtual_alternative()
{
    virtual_sequence_source source{{
        sw::face::six,
        sw::face::six,
        sw::face::six,
        sw::face::six,
        sw::face::six,
    }};

    const auto hand = sw::alternatives::initial_roll(source);
    for (const auto side : hand.values()) {
        require(side == sw::face::six, "virtual source should drive the same domain operation");
    }
}

void test_erased_alternative()
{
    sequence_source sequence{{
        sw::face::one,
        sw::face::two,
        sw::face::three,
        sw::face::four,
        sw::face::five,
    }};
    sw::alternatives::erased_face_source source = [&sequence] { return sequence(); };

    auto hand = sw::alternatives::initial_roll(source);
    const std::array<bool, sw::dice_per_hand> held{true, false, true, false, true};

    sequence_source replacement{{sw::face::six, sw::face::six}};
    source = [&replacement] { return replacement(); };
    hand = sw::alternatives::reroll(hand, sw::hold_view{held}, source);

    require(hand[0] == sw::face::one, "erased seam should preserve held die 0");
    require(hand[1] == sw::face::six, "erased seam should reroll die 1");
    require(hand[2] == sw::face::three, "erased seam should preserve held die 2");
    require(hand[3] == sw::face::six, "erased seam should reroll die 3");
    require(hand[4] == sw::face::five, "erased seam should preserve held die 4");
}

void test_urbg_alternative()
{
    std::minstd_rand engine{7};
    auto hand = sw::alternatives::initial_roll_urbg(engine);
    const std::array<bool, sw::dice_per_hand> held{false, false, true, false, false};
    const auto preserved = hand[2];

    hand = sw::alternatives::reroll_urbg(hand, sw::hold_view{held}, engine);

    require(hand[2] == preserved, "URBG alternative should preserve held dice");
    for (const auto side : hand.values()) {
        const auto numeric = sw::value(side);
        require(numeric >= 1 && numeric <= sw::sides_per_die, "URBG alternative returned invalid face");
    }
}

} // namespace

int main()
{
    try {
        test_semantic_callable_seam();
        test_hold_contract_is_fixed_extent_and_borrowed();
        test_uniform_source_produces_domain_faces();
        test_virtual_alternative();
        test_erased_alternative();
        test_urbg_alternative();
    } catch (const std::exception& error) {
        std::cerr << "yahtzee-randomness study failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

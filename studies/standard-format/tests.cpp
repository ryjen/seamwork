#include <cstdlib>
#include <format>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include <seamwork/studies/standard_format.hpp>

namespace sw = seamwork::studies::standard_format;

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

constexpr sw::diagnostic sample{
    .component = "net",
    .latency_ms = 23.45213,
    .code = 10,
};

constexpr std::string_view expected =
    "component=net      latency=23.45ms code=000A";

void test_direct_standard_format()
{
    require(
        sw::render_diagnostic(sample) == expected,
        "direct std::format should cover alignment, precision, and hexadecimal output"
    );
}

void test_caller_owned_output()
{
    std::string output = "report: ";
    sw::append_diagnostic(std::back_inserter(output), sample);

    require(
        output == std::string{"report: "} + std::string{expected},
        "std::format_to should write into caller-owned output"
    );
}

void test_runtime_format_behavior()
{
    require(
        sw::render_runtime(sw::diagnostic_pattern, sample) == expected,
        "std::vformat should support a genuinely runtime format string"
    );

    bool rejected = false;
    try {
        static_cast<void>(sw::render_runtime("{", sample));
    } catch (const std::format_error&) {
        rejected = true;
    }

    require(rejected, "invalid runtime format strings should fail with std::format_error");
}

void test_forwarding_wrapper_adds_no_semantics()
{
    const auto direct = std::format("{} {:04X}", "code", 10U);
    const auto wrapped = sw::alternatives::forwarding_format("{} {:04X}", "code", 10U);

    require(wrapped == direct, "thin forwarding wrapper should be behaviorally identical");
}

} // namespace

int main()
{
    try {
        test_direct_standard_format();
        test_caller_owned_output();
        test_runtime_format_behavior();
        test_forwarding_wrapper_adds_no_semantics();
    } catch (const std::exception& error) {
        std::cerr << "standard-format study failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#pragma once

#include <format>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>

namespace seamwork::studies::standard_format {

struct diagnostic {
    std::string_view component;
    double latency_ms{};
    unsigned code{};
};

inline constexpr std::string_view diagnostic_pattern =
    "component={:<8} latency={:.2f}ms code={:04X}";

[[nodiscard]] inline auto render_diagnostic(const diagnostic& value) -> std::string
{
    return std::format(
        "component={:<8} latency={:.2f}ms code={:04X}",
        value.component,
        value.latency_ms,
        value.code
    );
}

template <class Output>
inline auto append_diagnostic(Output output, const diagnostic& value) -> Output
{
    return std::format_to(
        output,
        "component={:<8} latency={:.2f}ms code={:04X}",
        value.component,
        value.latency_ms,
        value.code
    );
}

[[nodiscard]] inline auto render_runtime(
    std::string_view pattern,
    const diagnostic& value
) -> std::string
{
    return std::vformat(
        pattern,
        std::make_format_args(value.component, value.latency_ms, value.code)
    );
}

namespace alternatives {

template <class... Args>
[[nodiscard]] auto forwarding_format(
    std::format_string<Args...> pattern,
    Args&&... args
) -> std::string
{
    return std::format(pattern, std::forward<Args>(args)...);
}

} // namespace alternatives

} // namespace seamwork::studies::standard_format

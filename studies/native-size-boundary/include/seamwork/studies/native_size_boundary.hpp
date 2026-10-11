#pragma once

#include <cstddef>
#include <expected>
#include <span>
#include <utility>

namespace seamwork::studies::native_size_boundary {

enum class size_error_code {
    not_representable,
};

struct size_error {
    size_error_code code;
    std::size_t requested{};
};

[[nodiscard]] constexpr auto checked_native_count(std::size_t size)
    -> std::expected<int, size_error>
{
    if (!std::in_range<int>(size)) {
        return std::unexpected{size_error{
            .code = size_error_code::not_representable,
            .requested = size,
        }};
    }

    return static_cast<int>(size);
}

enum class write_error_code {
    size_not_representable,
    native_failure,
};

struct write_error {
    write_error_code code;
    std::size_t requested{};
    int native_result{};
};

class fake_native_sink
{
public:
    void fail_next() noexcept
    {
        next_result_ = -1;
        has_next_result_ = true;
    }

    void return_next(int result) noexcept
    {
        next_result_ = result;
        has_next_result_ = true;
    }

    [[nodiscard]] auto write(const std::byte*, int size) noexcept -> int
    {
        ++calls_;
        last_count_ = size;

        if (std::exchange(has_next_result_, false)) {
            return next_result_;
        }

        return size;
    }

    [[nodiscard]] auto calls() const noexcept -> unsigned
    {
        return calls_;
    }

    [[nodiscard]] auto last_count() const noexcept -> int
    {
        return last_count_;
    }

private:
    unsigned calls_{};
    int last_count_{};
    int next_result_{};
    bool has_next_result_{};
};

[[nodiscard]] inline auto write_once(
    std::span<const std::byte> bytes,
    fake_native_sink& sink
) -> std::expected<std::size_t, write_error>
{
    if (bytes.empty()) {
        return std::size_t{0};
    }

    const auto native_count = checked_native_count(bytes.size());
    if (!native_count) {
        return std::unexpected{write_error{
            .code = write_error_code::size_not_representable,
            .requested = bytes.size(),
        }};
    }

    const auto result = sink.write(bytes.data(), *native_count);
    if (result < 0) {
        return std::unexpected{write_error{
            .code = write_error_code::native_failure,
            .requested = bytes.size(),
            .native_result = result,
        }};
    }

    return static_cast<std::size_t>(result);
}

namespace alternatives {

[[nodiscard]] constexpr auto unchecked_native_count(std::size_t size) noexcept -> int
{
    return static_cast<int>(size);
}

} // namespace alternatives

} // namespace seamwork::studies::native_size_boundary

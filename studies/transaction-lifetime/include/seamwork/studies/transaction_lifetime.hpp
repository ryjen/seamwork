#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <scope>
#include <span>
#include <utility>

namespace seamwork::studies::transaction_lifetime {

enum class operation : std::uint8_t {
    begin,
    commit,
    rollback,
};

enum class transaction_error_code : std::uint8_t {
    begin_failed,
    commit_failed,
    rollback_failed,
    invalid_state,
};

struct transaction_error {
    transaction_error_code code;
};

class fake_backend
{
public:
    void fail_next_begin() noexcept
    {
        fail_begin_ = true;
    }

    void fail_next_commit() noexcept
    {
        fail_commit_ = true;
    }

    void fail_next_rollback() noexcept
    {
        fail_rollback_ = true;
    }

    [[nodiscard]] auto begin() noexcept -> std::expected<void, transaction_error>
    {
        record_event(operation::begin);

        if (active_) {
            return std::unexpected{transaction_error{
                .code = transaction_error_code::invalid_state,
            }};
        }

        if (std::exchange(fail_begin_, false)) {
            return std::unexpected{transaction_error{
                .code = transaction_error_code::begin_failed,
            }};
        }

        active_ = true;
        return {};
    }

    [[nodiscard]] auto commit() noexcept -> std::expected<void, transaction_error>
    {
        record_event(operation::commit);

        if (!active_) {
            return std::unexpected{transaction_error{
                .code = transaction_error_code::invalid_state,
            }};
        }

        if (std::exchange(fail_commit_, false)) {
            return std::unexpected{transaction_error{
                .code = transaction_error_code::commit_failed,
            }};
        }

        active_ = false;
        return {};
    }

    [[nodiscard]] auto rollback() noexcept -> std::expected<void, transaction_error>
    {
        record_event(operation::rollback);

        if (!active_) {
            return std::unexpected{transaction_error{
                .code = transaction_error_code::invalid_state,
            }};
        }

        if (std::exchange(fail_rollback_, false)) {
            return std::unexpected{transaction_error{
                .code = transaction_error_code::rollback_failed,
            }};
        }

        active_ = false;
        return {};
    }

    void record_unobserved_cleanup_failure(transaction_error error) noexcept
    {
        if (cleanup_failure_count_ < cleanup_failures_.size()) {
            cleanup_failures_[cleanup_failure_count_++] = error;
        }
    }

    [[nodiscard]] auto active() const noexcept -> bool
    {
        return active_;
    }

    [[nodiscard]] auto events() const noexcept -> std::span<const operation>
    {
        return std::span<const operation>{events_.data(), event_count_};
    }

    [[nodiscard]] auto cleanup_failures() const noexcept
        -> std::span<const transaction_error>
    {
        return std::span<const transaction_error>{
            cleanup_failures_.data(),
            cleanup_failure_count_,
        };
    }

private:
    void record_event(operation event) noexcept
    {
        if (event_count_ < events_.size()) {
            events_[event_count_++] = event;
        }
    }

    bool active_{};
    bool fail_begin_{};
    bool fail_commit_{};
    bool fail_rollback_{};
    std::array<operation, 16> events_{};
    std::size_t event_count_{};
    std::array<transaction_error, 4> cleanup_failures_{};
    std::size_t cleanup_failure_count_{};
};

class transaction_guard
{
public:
    transaction_guard(const transaction_guard&) = delete;
    auto operator=(const transaction_guard&) -> transaction_guard& = delete;

    transaction_guard(transaction_guard&& other) noexcept
        : backend_(std::exchange(other.backend_, nullptr)),
          active_(std::exchange(other.active_, false))
    {
    }

    auto operator=(transaction_guard&&) -> transaction_guard& = delete;

    ~transaction_guard() noexcept
    {
        rollback_on_abandonment();
    }

    [[nodiscard]] static auto start(fake_backend& backend)
        -> std::expected<transaction_guard, transaction_error>
    {
        auto result = backend.begin();
        if (!result) {
            return std::unexpected{result.error()};
        }

        return transaction_guard{backend};
    }

    [[nodiscard]] auto commit() noexcept -> std::expected<void, transaction_error>
    {
        if (!active_ || backend_ == nullptr) {
            return std::unexpected{transaction_error{
                .code = transaction_error_code::invalid_state,
            }};
        }

        auto result = backend_->commit();
        if (result) {
            active_ = false;
        }
        return result;
    }

    [[nodiscard]] auto rollback() noexcept -> std::expected<void, transaction_error>
    {
        if (!active_ || backend_ == nullptr) {
            return std::unexpected{transaction_error{
                .code = transaction_error_code::invalid_state,
            }};
        }

        auto result = backend_->rollback();
        if (result) {
            active_ = false;
        }
        return result;
    }

    [[nodiscard]] auto active() const noexcept -> bool
    {
        return active_;
    }

private:
    explicit transaction_guard(fake_backend& backend) noexcept
        : backend_(&backend), active_(true)
    {
    }

    void rollback_on_abandonment() noexcept
    {
        if (!active_ || backend_ == nullptr) {
            return;
        }

        auto result = backend_->rollback();
        if (!result) {
            backend_->record_unobserved_cleanup_failure(result.error());
        }

        active_ = false;
    }

    fake_backend* backend_{};
    bool active_{};
};

namespace alternatives {

class copyable_success_flag_transaction
{
public:
    explicit copyable_success_flag_transaction(fake_backend& backend) noexcept
        : backend_(&backend)
    {
    }

    copyable_success_flag_transaction(const copyable_success_flag_transaction&) = default;
    auto operator=(const copyable_success_flag_transaction&)
        -> copyable_success_flag_transaction& = default;

    ~copyable_success_flag_transaction() noexcept
    {
        if (backend_ == nullptr || !backend_->active()) {
            return;
        }

        auto result = successful_ ? backend_->commit() : backend_->rollback();
        if (!result) {
            backend_->record_unobserved_cleanup_failure(result.error());
        }
    }

    void set_successful(bool successful) noexcept
    {
        successful_ = successful;
    }

private:
    fake_backend* backend_{};
    bool successful_{};
};

[[nodiscard]] inline auto run_with_scope_exit(fake_backend& backend)
    -> std::expected<void, transaction_error>
{
    auto begun = backend.begin();
    if (!begun) {
        return std::unexpected{begun.error()};
    }

    auto rollback = std::scope_exit{[&backend]() noexcept {
        auto result = backend.rollback();
        if (!result) {
            backend.record_unobserved_cleanup_failure(result.error());
        }
    }};

    auto committed = backend.commit();
    if (!committed) {
        return std::unexpected{committed.error()};
    }

    rollback.release();
    return {};
}

} // namespace alternatives

} // namespace seamwork::studies::transaction_lifetime

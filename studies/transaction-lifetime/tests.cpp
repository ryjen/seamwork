#include <cstdlib>
#include <array>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <seamwork/studies/transaction_lifetime.hpp>

namespace sw = seamwork::studies::transaction_lifetime;

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

template <std::size_t N>
void require_events(
    const sw::fake_backend& backend,
    const std::array<sw::operation, N>& expected,
    std::string_view message
)
{
    require(std::ranges::equal(backend.events(), expected), message);
}

void test_explicit_commit()
{
    sw::fake_backend backend;

    {
        auto started = sw::transaction_guard::start(backend);
        require(started.has_value(), "transaction should start");

        auto tx = std::move(*started);
        const auto committed = tx.commit();
        require(committed.has_value(), "commit should succeed explicitly");
        require(!tx.active(), "successful commit should finish guard lifecycle");
    }

    require_events(
        backend,
        std::array{sw::operation::begin, sw::operation::commit},
        "successful scope should not add a rollback"
    );
}

void test_abandonment_rolls_back()
{
    sw::fake_backend backend;

    {
        auto started = sw::transaction_guard::start(backend);
        require(started.has_value(), "transaction should start");
        auto tx = std::move(*started);
        require(tx.active(), "started transaction should be active");
    }

    require_events(
        backend,
        std::array{sw::operation::begin, sw::operation::rollback},
        "abandoned transaction should roll back"
    );
}

void test_exception_path_rolls_back()
{
    sw::fake_backend backend;

    try {
        auto started = sw::transaction_guard::start(backend);
        require(started.has_value(), "transaction should start");
        auto tx = std::move(*started);
        throw std::runtime_error{"work failed"};
    } catch (const std::runtime_error&) {
    }

    require_events(
        backend,
        std::array{sw::operation::begin, sw::operation::rollback},
        "exception path should roll back during unwinding"
    );
}

void test_commit_failure_is_observable_then_rolls_back()
{
    sw::fake_backend backend;
    backend.fail_next_commit();

    {
        auto started = sw::transaction_guard::start(backend);
        require(started.has_value(), "transaction should start");
        auto tx = std::move(*started);

        const auto committed = tx.commit();
        require(!committed, "injected commit failure should be returned");
        require(
            committed.error().code == sw::transaction_error_code::commit_failed,
            "commit failure should retain typed reason"
        );
        require(tx.active(), "failed commit should leave transaction active for rollback");
    }

    require_events(
        backend,
        std::array{
            sw::operation::begin,
            sw::operation::commit,
            sw::operation::rollback,
        },
        "failed commit should be followed by fail-safe rollback"
    );
}

void test_repeat_transition_is_rejected()
{
    sw::fake_backend backend;
    auto started = sw::transaction_guard::start(backend);
    require(started.has_value(), "transaction should start");
    auto tx = std::move(*started);

    require(tx.commit().has_value(), "first commit should succeed");

    const auto second_commit = tx.commit();
    require(!second_commit, "second commit should fail");
    require(
        second_commit.error().code == sw::transaction_error_code::invalid_state,
        "repeat commit should report invalid lifecycle state"
    );

    const auto rollback = tx.rollback();
    require(!rollback, "rollback after commit should fail");
    require(
        rollback.error().code == sw::transaction_error_code::invalid_state,
        "rollback after commit should report invalid lifecycle state"
    );
}

void test_move_transfers_rollback_ownership()
{
    sw::fake_backend backend;

    {
        auto started = sw::transaction_guard::start(backend);
        require(started.has_value(), "transaction should start");

        auto first = std::move(*started);
        auto second = std::move(first);

        // transaction_guard explicitly clears active_ via std::exchange on move.
        // NOLINTNEXTLINE(bugprone-use-after-move)
        require(!first.active(), "moved-from guard should release lifecycle ownership");
        require(second.active(), "moved-to guard should own active transaction");
        require(second.commit().has_value(), "moved-to guard should commit");
    }

    require_events(
        backend,
        std::array{sw::operation::begin, sw::operation::commit},
        "only moved-to guard should finalize transaction"
    );
}

void test_cleanup_failure_does_not_throw_from_destructor()
{
    sw::fake_backend backend;
    backend.fail_next_rollback();

    {
        auto started = sw::transaction_guard::start(backend);
        require(started.has_value(), "transaction should start");
        auto tx = std::move(*started);
    }

    require(
        backend.cleanup_failures().size() == 1,
        "automatic rollback failure should remain observable outside destructor"
    );
    require(
        backend.cleanup_failures().front().code == sw::transaction_error_code::rollback_failed,
        "cleanup observer should retain rollback failure"
    );
}

void test_copyable_success_flag_has_conflicting_owners()
{
    sw::fake_backend backend;
    require(backend.begin().has_value(), "baseline transaction should begin");

    {
        sw::alternatives::copyable_success_flag_transaction outer{backend};
        outer.set_successful(true);

        {
            auto conflicting_copy = outer;
            conflicting_copy.set_successful(false);
        }

        require(!backend.active(), "copy destruction should finalize shared backend state");
    }

    require_events(
        backend,
        std::array{sw::operation::begin, sw::operation::rollback},
        "copy destruction order can override another copy's commit intent"
    );
}

#if __has_include(<scope>)
void test_scope_exit_alternative()
{
    sw::fake_backend backend;
    const auto result = sw::alternatives::run_with_scope_exit(backend);

    require(result.has_value(), "scope_exit alternative should commit");
    require_events(
        backend,
        std::array{sw::operation::begin, sw::operation::commit},
        "successful explicit commit should release scope-exit rollback"
    );
}
#endif

} // namespace

int main()
{
    try {
        test_explicit_commit();
        test_abandonment_rolls_back();
        test_exception_path_rolls_back();
        test_commit_failure_is_observable_then_rolls_back();
        test_repeat_transition_is_rejected();
        test_move_transfers_rollback_ownership();
        test_cleanup_failure_does_not_throw_from_destructor();
        test_copyable_success_flag_has_conflicting_owners();
#if __has_include(<scope>)
        test_scope_exit_alternative();
#endif
    } catch (const std::exception& error) {
        std::cerr << "transaction-lifetime study failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

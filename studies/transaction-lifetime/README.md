# Transaction lifetime study

Issue #58 extracts a lifecycle question from archived `libcoda-db` without importing a database backend.

The historical transaction object was copyable, shared its backend/session state, kept a per-object `successful_` flag, and committed or rolled back from its destructor. Commit/rollback could throw.

That makes the transaction result depend on copy/destruction order and gives commit failure no safe destructor-time result channel.

## Selected design

Use a **move-only transaction guard** with:

- explicit `start(...)`;
- explicit `commit() -> std::expected<void, transaction_error>`;
- explicit optional `rollback()`;
- rollback-on-abandonment in a `noexcept` destructor;
- failed commit leaves the guard active so scope exit still attempts rollback.

```cpp
auto started = transaction_guard::start(backend);
if (!started) return ...;

auto tx = std::move(*started);

// work

auto committed = tx.commit();
if (!committed) return ...; // destructor attempts rollback
```

Commit is therefore an observable operation, not a success flag consumed later by a destructor.

## Destructor rollback failure

Automatic cleanup cannot return an error to an object whose lifetime is ending.

The deterministic backend records an unobserved cleanup failure to demonstrate the required production policy: a real integration needs a non-throwing observability path (logging/metrics/health state) for destructor-time rollback failure.

The destructor itself never throws.

## C++23 `std::scope_exit` alternative

For a purely local composition boundary, this is also clear:

```cpp
begin();
auto rollback = std::scope_exit([&] { rollback_noexcept(); });

auto committed = commit();
if (!committed) return error;

rollback.release();
```

The study keeps it as a concrete alternative. The named guard is preferred when transaction ownership/lifecycle itself is part of the interface and may move between local scopes.

See [QART](qart.md).

## Build

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_transaction_lifetime
./build/dev/studies/transaction-lifetime/seamwork_transaction_lifetime_example
```

## Promotion status

**Study only.**

No database or generic transaction framework is introduced.

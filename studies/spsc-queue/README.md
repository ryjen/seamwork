# Bounded SPSC queue study

Issue #73 is driven by a concrete external consumer seam in `ryjen/dsp`.

That repository independently contains two fixed-capacity single-producer/single-consumer queues:

- a performance-observation queue;
- a realtime MIDI inbox.

The payloads differ, but the sequence counters, capacity, memory ordering, full/empty behavior, and no-allocation requirements are effectively the same.

This study asks whether that repeated concurrency contract deserves one small generic type.

It does **not** promote a queue component and does not make `ryjen/dsp` depend on Seamwork.

## Selected interface

```cpp
template <class T, std::size_t Capacity>
using queue = ...;

bool try_push(const T&) noexcept;
bool try_push(T&&) noexcept;
std::optional<T> try_pop() noexcept;
```

The copy overload exists only when copying `T` is nothrow.

The value type must be nothrow move-constructible and nothrow destructible because the studied realtime consumers cannot tolerate an exception path inside queue publication/consumption.

## Contract

- exactly one producer thread;
- exactly one consumer thread;
- fixed queue-owned storage;
- power-of-two capacity;
- no blocking;
- no overwrite when full;
- `try_push` returns `false` when full;
- `try_pop` returns `std::nullopt` when empty;
- FIFO order;
- move-only payloads are supported;
- queue operations themselves allocate nothing.

The queue is non-copyable and non-movable because transferring a live synchronization object creates a separate lifetime problem that the current consumers do not need.

## Memory ordering

Producer:

1. load its own write sequence with `relaxed`;
2. load the consumer's read sequence with `acquire`;
3. stop if the queue is full;
4. construct the payload in the selected slot;
5. publish the next write sequence with `release`.

Consumer:

1. load its own read sequence with `relaxed`;
2. load the producer's write sequence with `acquire`;
3. stop if the queue is empty;
4. move the payload out and destroy the stored object;
5. publish the next read sequence with `release`.

Under the SPSC precondition, only the producer writes `write_sequence` and only the consumer writes `read_sequence`.

The release/acquire pairs publish slot construction to the consumer and slot retirement to the producer before either side reuses storage.

The study does not claim this ordering is universally optimal. It records why it is sufficient for this exact one-producer/one-consumer contract.

## Sequence wraparound

Unsigned sequence subtraction is deliberate:

```cpp
static_cast<Sequence>(write - read)
```

The outstanding distance can never exceed `Capacity`.

The slot mapping also exposes an additional invariant:

```cpp
slot = sequence % Capacity
```

When the unsigned sequence counter wraps, that mapping remains continuous only when `Capacity` divides the counter modulus. For binary unsigned counters, requiring a power-of-two capacity makes that relationship explicit.

The first study implementation incorrectly allowed arbitrary capacities. A `uint8_t` wraparound test with capacity 3 failed in CI and exposed the assumption. The selected interface now requires a power-of-two capacity.

Tests instantiate the internal implementation with `std::uint8_t` and capacity 4, then repeatedly fill/drain it so both counters wrap many times without requiring billions of operations.

The supported public alias uses `std::uint32_t`.

## Alternatives

See [QART](qart.md) for comparison with:

- keeping domain-specific duplicated queues;
- mutex-backed queues;
- overwrite/drop-oldest rings;
- mature external lock-free queue libraries.

## External adoption

A successful study would make a small generic queue technically plausible for the two DSP uses.

That is **not** enough by itself to promote a Seamwork component. Both consumers are still in one realtime DSP repository.

Before promotion, prefer either:

- a second independent consumer domain/repository; or
- unusually strong standalone lifecycle/versioning evidence.

## Promotion status

**Study only.**


## ThreadSanitizer qualification

The queue's correctness claim depends on cross-thread publication, so concurrent stress tests are additionally qualified under Clang ThreadSanitizer.

The dedicated `.github/workflows/tsan.yml` workflow is intentionally separate from normal CI:

- it runs manually or when the SPSC study / TSan workflow changes;
- it builds only `seamwork_spsc_queue_test`;
- it enables `-fsanitize=thread`;
- unrelated Seamwork pull requests do not pay the sanitizer cost.

This adds dynamic data-race evidence without changing the queue contract or promoting it into a component.

The normal test binary still performs its explicit no-allocation check by temporarily replacing global `new/delete`. The TSan build disables only that test instrumentation because ThreadSanitizer owns the same allocator-interception symbols. Queue behavior tests and the concurrent stress case still run under TSan.

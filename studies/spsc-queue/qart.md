# QART: bounded SPSC queue

## Question

When two realtime consumers repeat the same fixed-capacity SPSC sequence-counter logic, should the synchronization contract remain duplicated or become a generic value type?

## Consumer evidence

`ryjen/dsp` independently implements the same queue shape for:

1. bounded performance observations crossing from realtime analysis toward non-realtime handling;
2. timestamped MIDI messages crossing into the realtime control path.

Both queues use capacity 64, fixed arrays, unsigned sequence counters, acquire/release publication, fail-on-full semantics, optional pop, and allocation-free steady-state tests.

The duplication is structural rather than naming-level.

## Alternatives

### A. Keep domain-specific queues

This is the current DSP baseline.

Advantages:

- code stays local to each domain;
- payload construction can be specialized freely;
- no shared abstraction dependency.

Costs:

- memory-order reasoning is duplicated;
- wraparound assumptions are duplicated;
- correctness fixes/tests must be repeated.

This remains reasonable if the shared queue contract turns out to be less stable than it appears.

### B. Generic fixed-capacity SPSC queue

Selected study hypothesis.

The generic parameters are deliberately limited to payload type and capacity.

It does not generalize overflow policy, producer count, consumer count, waiting behavior, allocation strategy, or scheduler integration.

The result is smaller than a generic "channel" abstraction.

### C. Mutex + standard queue/deque

For ordinary application code this can be easier to prove and maintain.

It is not equivalent for the motivating DSP consumers because:

- blocking locks are prohibited in the realtime callback;
- a standard deque may allocate;
- the current contract requires bounded fixed storage.

Use the simpler mutex-backed design whenever those realtime constraints do not exist.

### D. Overwrite/drop-oldest ring

A telemetry system may prefer newest data over backpressure.

The motivating DSP queues do not: callers observe `false` from push and can decide how to account for a full queue.

Encoding overwrite behavior would silently change that contract.

### E. Mature external lock-free queue

A mature library becomes preferable when requirements expand to areas such as:

- MPMC;
- blocking/wait integration;
- dynamic capacities;
- cache-topology tuning;
- extensive architecture-specific optimization;
- independently maintained lock-free correctness evidence.

The study should not compete with that scope.

## Storage

Each slot is a `std::optional<T>`.

This gives explicit object lifetime inside fixed storage and allows move-only payloads without requiring `T` to be default-constructible.

No heap allocation is required by `optional` itself.

## Payload requirements

The selected implementation requires nothrow move construction and destruction.

That reflects the motivating realtime contract: after a slot is published or consumed, there is no sensible exception recovery protocol inside the ring.

Copy push is available only for nothrow-copyable payloads.

These constraints are intentionally stricter than a general-purpose queue.

## Progress claims

The queue uses no locks or blocking calls and requires lock-free sequence atomics at compile time.

The study does **not** make a formal wait-free or lock-free progress guarantee for the entire operation, because that would require stronger architecture/toolchain evidence than the current project has established.

## Wraparound

Write and read sequences are monotonically increasing unsigned counters modulo the sequence type's range.

Fullness uses the unsigned modular distance:

```cpp
static_cast<Sequence>(write - read) >= Capacity
```

Because the producer is never allowed to advance more than `Capacity` slots ahead of the consumer, the outstanding distance remains representable modulo the sequence range.

However, slot selection also uses `sequence % Capacity`. The initial generic study allowed arbitrary capacities. CI deliberately instantiated an 8-bit sequence with capacity 3; when the sequence wrapped from 255 to 0, the slot mapping did not advance continuously because 3 does not divide the 256-value sequence modulus.

That failure exposed a hidden assumption already satisfied by the motivating DSP queues: their capacity is 64, a power of two.

The selected study now requires power-of-two capacity. For binary unsigned sequence types, the sequence modulus is also a power of two, so the slot mapping remains continuous across wrap.

The test-only sequence parameter uses `uint8_t` with capacity 4 to qualify repeated wraparound in bounded time.

## Recommendation

The generic SPSC shape is justified as a study because the same correctness-sensitive synchronization contract has already been implemented twice in a real external consumer repository.

Do not promote it yet.

The useful question for future adoption is not "can this be generic?" It clearly can. The question is whether centralizing the contract reduces total risk without making realtime consumers depend on an abstraction whose lifecycle is broader than their own.

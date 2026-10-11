# Seamwork study catalog and evidence coverage

This catalog summarizes what Seamwork has actually learned from maintained studies and consumers.

It is an evidence map, not a maturity score. A selected technique is preferred only for the context studied; negative results and deliberate non-promotions are first-class outcomes.

## Evidence catalog

| Study / component | Historical pressure | Primary design question | C++ mechanism(s) | Selected outcome | Promotion result | Evidence |
| --- | --- | --- | --- | --- | --- | --- |
| [Yahtsee randomness seam](../studies/yahtsee-randomness/README.md) | Generic dice behavior and random generation were coupled to consumers | How should a small domain depend on randomness without hiding ownership or cost? | Concepts, callable templates, URBG, virtual/type-erased alternatives | Semantic callable `face_source` with caller-owned source; direct alternatives remain comparison evidence | Randomness behavior remains study-only; stable face/hand values were later promoted separately | Deterministic source tests, virtual/type-erasure/URBG alternatives, real Yahtsee consumer |
| [Yahtsee recoverable-error boundary](../studies/yahtsee-expected/README.md) | Raw category/hand input mixed validation and scoring concerns | Where should recoverable failure exist, and where should it disappear after validation? | `std::expected`, exceptions/optional comparison, validated domain values | Use explicit recoverable results at parsing/validation boundaries; make scoring total once invariants hold | No Seamwork-wide error framework; stable hand value later promoted | Invalid category/hand tests, scoring tests, composed consumer |
| [Yahtsee typed turn state](../studies/yahtsee-turn-state/README.md) | Flag-driven mutable state permitted awkward/invalid combinations | Can the type system represent only meaningful turn states? | `std::variant`, state-specific values, explicit transition errors | Closed typed state machine with explicit transitions | Study only | Deterministic transition tests and composed turn consumer |
| [Standard formatting](../studies/standard-format/README.md) | Historical formatter owned grammar, parser, binder, renderer, and error policy | Does C++23 remove the need for a project formatter abstraction? | `std::format`, `std::format_to`, `std::vformat` | Use standard formatting directly; caller-owned output uses `format_to` | **Promote nothing**; custom formatter abstraction deleted from modern design vocabulary | Alignment/precision/hex tests, runtime-format failure test, forwarding-wrapper negative alternative |
| [Borrowed frame parsing](../studies/borrowed-frame/README.md) | Raw pointer/length buffers and parser/transport coupling obscured borrowing and bounds | How should a pure parser express borrowing, limits, malformed input, and retention? | `std::span`, ranges/`borrowed_range`, `std::expected` | Borrow contiguous caller storage; reject temporary owners; copy only through explicit retention | Study only; no networking/parser framework | Truncation/limit tests, compile-time lifetime constraints, explicit retain-copy evidence |
| [Transaction lifetime](../studies/transaction-lifetime/README.md) | Copyable transaction objects and commit-on-destruction made success/failure lifecycle implicit | How should one active transaction own commit/rollback responsibility? | Move-only RAII, `std::expected`, conditional `std::scope_exit` comparison | Explicit commit; rollback by default; non-throwing destruction; one lifecycle owner | Study only; no DB/transaction framework | Commit/rollback/failure/move/unwind tests; portability evidence for missing `<scope>` |
| [Authority vs transport identity](../studies/authority-transport/README.md) | DNS resolution risked replacing caller authority with routing address at TLS boundary | How can trust identity and routing identity be impossible to confuse accidentally? | Semantic value types, immutable value-oriented plan | Preserve caller authority separately from resolved endpoint; type boundaries reject substitution | Study only; no networking framework | Compile-time substitution checks, multi-candidate and IP-literal tests |
| [Native size boundary](../studies/native-size-boundary/README.md) | `size_t` C++ buffers crossed narrower native integer APIs | How should representability and partial native results remain visible? | `std::span`, `std::in_range`, `std::expected` | Validate before narrowing; preserve partial counts; no hidden retry/chunking | Study only | `INT_MAX` boundary tests, native failure/partial write/zero-length tests |
| [Percent decoding](../studies/percent-decoding/README.md) | Legacy decoder performed unchecked `%HH` lookahead and accepted malformed hex | How should malformed external encoding fail without out-of-range access? | Bounds-first parsing, `std::expected`, `std::bit_cast`, explicit policy enum | Validate range before lookahead; reject invalid hex; make `+` interpretation explicit | Study only; no URI/parser framework | Truncated/invalid/mixed-case/NUL/high-byte tests, exception and lenient alternatives |
| [Structured delayed work](../studies/structured-delayed-work/README.md) | Detached delayed threads had no owner, cancellation, join, or typed duration | How should one delayed operation express lifetime and cooperative cancellation? | `std::jthread`, `std::stop_token`, `std::stop_callback`, `std::chrono`, constrained callbacks | One move-only owner; stop-aware delay; destructor joins; callback is explicitly `noexcept` | Study only; no scheduler/executor/thread framework | Cancellation/destruction/move/cooperative-stop tests without wall-clock sleeps |
| [Bounded SPSC queue](../studies/spsc-queue/README.md) | Two realtime DSP consumers independently repeated the same fixed-capacity sequence-counter queue | Should one correctness-sensitive SPSC contract become a narrow generic value type? | Fixed `std::array<std::optional<T>>` storage, atomics, acquire/release publication, unsigned modular sequence arithmetic | Generic only across payload/capacity; fail-on-full, optional pop, no blocking/allocation; strict SPSC precondition | Study only; external adoption/promotion deliberately deferred | Two consumer-shaped payloads, no-allocation test, move-only payload, concurrent FIFO stress, bounded `uint8_t` wraparound qualification |
| [Yahtsee domain component](../components/yahtsee-domain/README.md) | Two consumers repeated the same five-face domain conversion | When has a shared value type earned promotion? | Strong enum/value type, fixed-size aggregate, `std::expected` raw validation | Promote only stable Yahtsee face + five-face hand facts | **Promoted component** | Two independent consumers reproduced the adapter before promotion |
| [Composed Yahtsee turn](../examples/yahtsee-turn/README.md) | Study-local abstractions needed real composition pressure | Which seams remain clear when randomness, turn lifecycle, and scoring are used together? | Composition root, explicit adapters, typed state | Turn state became single hold authority; first consumer deliberately deferred hand promotion | Consumer/evidence, not component | Exposed duplicate hold state and first hand-conversion occurrence |
| [Yahtsee scorecard](../examples/yahtsee-scorecard/README.md) | One consumer was insufficient evidence for shared domain promotion | Does integration friction repeat in an independent workflow? | Reuse of study boundaries across multiple turns/categories | Repeated hand adapter supplied second-consumer evidence for narrow domain promotion | Consumer/evidence, not framework | Two-turn deterministic scorecard and repeated adapter evidence |

## Cross-study coverage

### Strongly covered

These areas have repeated or mutually reinforcing evidence.

| Dimension | Evidence | Current conclusion |
| --- | --- | --- |
| Ownership and lifetime | Borrowed frame, transaction guard, structured delayed work, Yahtsee hold ownership | Prefer explicit owners and explicit borrowing; make cleanup/cancellation responsibility visible in the type/interface |
| Value semantics and semantic types | Yahtsee domain, authority/transport identities, typed turn states | Small domain-specific values can remove invalid substitutions without requiring generic frameworks |
| Recoverable boundary errors | Yahtsee validation, frame parsing, native size adaptation, percent decoding, transaction operations | `std::expected` is useful at recoverable trust/operation boundaries; do not spread error channels into already-valid total operations |
| Trust-boundary parsing and adaptation | Borrowed frame, percent decoding, native size boundary, authority/transport identity | Validate before interpretation/narrowing; keep provenance and limits explicit |
| Fail-safe cleanup | Transaction lifetime and structured delayed work | Destruction should clean up safely, but success operations whose failure matters should remain explicit |
| Evidence-driven non-promotion | Formatting, frame parsing, transaction, concurrency, networking slices | A successful study may end with no reusable Seamwork component |

### Partially covered

These have useful evidence but not enough repetition to generalize broadly.

| Dimension | Evidence | Limitation |
| --- | --- | --- |
| Invalid-state modeling | Yahtsee typed turn state | One compact state machine; no evidence for a general state-machine abstraction |
| Dependency inversion / polymorphism | Yahtsee randomness seam | Strong comparison in one small domain; not a universal preference for concepts/templates |
| Native/C boundary adaptation | Native size boundary plus historical network evidence | Integer representation is covered; broader native handle/error models remain dependency-specific |
| Concurrency / cancellation | Structured delayed work plus bounded SPSC queue | Evidence now covers one owned cancellable task and one non-blocking producer/consumer handoff; still no evidence for executors, thread pools, schedulers, MPMC, or general channels |
| Standard-library replacement | Formatting, `std::variant`, `std::jthread`-era redesign | Strong examples, but replacement remains problem-specific |
| Practical promotion evidence | Yahtsee domain | Only one component has completed the two-consumer promotion path |
| Practical consumer diversity | Yahtsee turn + scorecard | Both are in the same domain; non-Yahtsee studies are currently focused examples rather than multi-feature applications |
| Ranges as API design | Borrowed-frame `borrowed_range` refinement | Ranges are used for a lifetime contract, not yet explored as a broader consumer-facing algorithm/pipeline interface |

## Intentionally deferred

These are not current backlog items merely because C++ offers relevant mechanisms.

- coroutine framework or generic async result model;
- executor, thread pool, scheduler, or event loop;
- generic networking client/server stack;
- generic database/query abstraction;
- custom formatting library;
- generic dice/game toolkit;
- generic parser framework;
- JSON wrapper;
- terminal framework;
- logging framework;
- arbitrary-precision arithmetic component.

They should become studies only when a believable consumer creates a concrete interface pressure.

## Evidence gaps

### 1. Pure-parser fuzz qualification — addressed by #71

Seamwork has two deterministic, transport-independent parsers at untrusted-input boundaries:

- [borrowed frame](../studies/borrowed-frame/README.md);
- [percent decoding](../studies/percent-decoding/README.md).

#71 adds opt-in Clang/libFuzzer harnesses with ASan/UBSan, small seed corpora, and a bounded path-scoped smoke workflow.

The fuzzers are intentionally outside normal CTest and the required GCC/Clang matrix. They qualify arbitrary inputs only when the fuzz surface or either parser changes, while longer local fuzz sessions remain opt-in.

This closes the main active adversarial-testing gap without introducing live networking or a generic fuzz framework.

### 2. Performance / allocation evidence — intentionally sparse

Most current decisions are based on contract clarity, correctness, ownership, or maintenance surface rather than performance claims.

That is appropriate. No benchmark should be added simply to fill a matrix cell.

Add measurement only when a future recommendation depends on cost—for example, a proposed owned-vs-borrowed interface where copying materially affects a real consumer.

### 3. Multi-consumer evidence outside Yahtsee

The bounded-SPSC study is the first Seamwork study triggered by repeated interface/correctness pressure in a maintained external domain: `ryjen/dsp`.

That improves consumer diversity, but it does not yet justify a Seamwork component. Both motivating SPSC consumers still live in the same repository and realtime-DSP lifecycle.

Keep the promotion bar unchanged: observe whether another independent domain repeats the same contract before centralizing it as a reusable component.

### 4. Result-bearing asynchronous work

Structured delayed work intentionally constrains callbacks to `noexcept` and does not choose an exception/result channel.

A future real consumer that needs asynchronous results could compare futures/promises, callbacks, or coroutine-based results. Without that consumer, the topic remains deferred.

## Testing coverage

Current studies rely primarily on deterministic tests and compiler-matrix qualification.

Useful testing evidence already includes:

- deterministic fake sources/backends/transports;
- compile-time type/constraint assertions;
- boundary and malformed-input cases;
- negative alternatives;
- GCC + Clang qualification;
- prior historical fuzzing evidence in archived formatter work.

Active evidence methods now also include path-scoped libFuzzer qualification with ASan/UBSan for the two pure parser trust boundaries.

## Promotion discipline

The Yahtsee domain component remains the reference example for promotion:

```text
study-local friction
  -> first consumer exposes duplication
  -> keep local while evidence is weak
  -> second independent consumer repeats the seam
  -> promote only stable domain facts
```

This is intentionally stricter than deduplicating code whenever two files look similar.

## Next priority

After #71, **do not manufacture another abstraction study merely to keep the queue busy**.

The strongest remaining gaps are intentionally conditional:

- add performance measurement only when a recommendation makes a real cost claim;
- revisit asynchronous result channels only when a consumer needs result-bearing work;
- consider another promotion only after independent consumers repeat the same stable seam;
- continue parser fuzz qualification when those trust-boundary surfaces change.

The next design study should therefore begin with concrete consumer pressure or a newly demonstrated defect, not an uncovered C++ feature.

# Seamwork

[![CI](https://github.com/ryjen/seamwork/actions/workflows/ci.yml/badge.svg?branch=development)](https://github.com/ryjen/seamwork/actions/workflows/ci.yml)
[![License](https://img.shields.io/:license-mit-blue.svg)](http://ryjen.mit-license.org)

**Modern C++23 through interface and architecture refinement.**

Seamwork is a practical laboratory for exploring how modern C++ can produce interfaces that are expressive, maintainable, difficult to misuse, and explicit about ownership, lifetime, cost, and failure.

The project began as `libcoda`, a C++ utility toolkit used to learn modern C++ and experiment with cleaner APIs. The re-charter keeps that interest in interface quality while dropping the goal of being a general-purpose utility library.

Reusable components may emerge from the work, but the primary artifacts are the design studies, practical consumers, tests, measurements, and documented trade-offs that justify them.

## Project direction

Seamwork focuses on:

- interface refinement;
- C++23 language and standard-library facilities used deliberately;
- clean architectural boundaries without ceremony;
- pragmatic SOLID principles;
- explicit ownership, lifetime, error, and resource semantics;
- realistic examples rather than syntax-only demonstrations;
- testing, static analysis, sanitizers, fuzzing, and benchmarks as design evidence;
- comparing alternatives instead of assuming one C++ idiom is universally best.

See:

- [Project charter](docs/charter.md)
- [Design principles](docs/principles.md)
- [Refinement workflow](docs/refinement-workflow.md)
- [Re-charter and migration plan](docs/migration/seamwork-rebrand.md)

## Architecture

The existing architecture work remains useful as migration guidance:

- [System context](docs/architecture/0001-system-context.md)
- [Layered architecture and SOLID guidance](docs/architecture/0002-layered-architecture.md)
- [Error handling policy](docs/architecture/0003-error-handling.md)
- [Dependency policy](docs/architecture/0004-dependency-policy.md)
- [ADR 0002: transition toward a single Seamwork repository](docs/adr/0002-seamwork-topology.md)

Legacy `libcoda` components are archived design and security evidence, not supported Seamwork components. Their pinned source revisions are recorded in the [migration record](docs/migration/seamwork-rebrand.md).

## C++23 baseline

C++23 is the baseline for active Seamwork code.

The rule is:

> use the simplest mechanism that preserves the desired semantics, and use C++23 where it makes the interface or architecture meaningfully better.

Historical components retain their original language requirements in their independent repositories and Git history. They are not built or shipped by Seamwork.

## Practical consumers

The original C++ Yahtsee implementation survives on the archived `ryjen/yahtsee` repository's `original` branch. It is used as historical baseline evidence, not imported wholesale.

A typical Seamwork study moves through:

```text
problem
  -> straightforward implementation
  -> identify friction / coupling / ambiguity
  -> compare alternatives
  -> refine with appropriate C++23 mechanisms
  -> test / analyze / benchmark
  -> validate with a real consumer
  -> optionally promote to a reusable component
```

Promotion is optional. Demonstrating why an abstraction should not exist is a valid result.

## Current studies

- [Yahtsee randomness seam](studies/yahtsee-randomness/README.md) — compares a semantic callable/concept, runtime interface, type erasure, and direct URBG dependency against a focused C++23 consumer.
- [Yahtsee `std::expected` boundary](studies/yahtsee-expected/README.md) — compares exceptions, `std::optional`, and `std::expected` for recoverable parsing/validation, then removes the error channel once domain invariants are established.
- [Yahtsee typed turn state](studies/yahtsee-turn-state/README.md) — compares mutable flag-driven state with a closed `std::variant` of state-specific values and explicit transition errors.
- [Standard formatting](studies/standard-format/README.md) — revisits the archived formatter and tests whether direct `std::format`/`std::format_to` can delete the project-specific abstraction entirely.
- [Borrowed frame parsing](studies/borrowed-frame/README.md) — isolates a historical network byte-boundary problem and compares raw pointer/length, ownership, and borrowed `std::span` semantics with bounded parsing.
- [Transaction lifetime](studies/transaction-lifetime/README.md) — revisits archived DB lifecycle semantics and compares copyable commit-on-destruction, a move-only explicit-commit guard, and C++23 `std::scope_exit`.
- [Authority vs transport identity](studies/authority-transport/README.md) — keeps caller authentication identity distinct from resolved routing addresses so resolver output cannot silently replace the authority being verified.
- [Native size boundary](studies/native-size-boundary/README.md) — checks `size_t -> int` representability before native byte I/O and keeps partial-write/retry policy explicit.
- [Percent decoding](studies/percent-decoding/README.md) — replaces unchecked `%HH` lookahead with bounds-first ASCII hex validation, typed malformed-input errors, and explicit `+` policy.

- [Structured delayed work](studies/structured-delayed-work/README.md) — replaces detached delayed threads with move-only `std::jthread` ownership, strongly typed duration, and cooperative stop semantics.

Studies are evidence and design work first. They are not automatically promoted into reusable Seamwork components.

## Promoted components

- [Yahtsee domain](components/yahtsee-domain/README.md) — the first promoted component, containing only the stable six-face/five-value domain contract and raw validation demonstrated by two independent consumers.

Promotion remains exceptional. Random generation, scoring, hold state, turn lifecycle, and scorecards remain outside the component.

## Integrated examples

- [Composed Yahtsee turn](examples/yahtsee-turn/README.md) — combines the randomness, typed turn-state, and scoring/error-boundary studies in one deterministic consumer and records the integration friction instead of hiding it behind new abstractions.
- [Yahtsee scorecard](examples/yahtsee-scorecard/README.md) — independently reuses the same study seams across two turns and score accumulation to test whether repeated adapter friction is real enough to justify promotion.

## Security

The existing security work remains part of the evidence base:

- [Historical aggregate threat model and release-hardening checklist](docs/security/threat-model.md)
- [Legacy `libcoda-net` threat model](docs/security/libcoda-net.md)
- [Legacy `libcoda-db` threat model](docs/security/libcoda-db.md)
- [Legacy `libcoda-format` threat model](docs/security/libcoda-format.md)

Security is treated as part of interface design: trust boundaries, input validation, resource bounds, error behavior, and secret handling should be explicit in the contract.

## Building

The default build is intentionally small and does not require the archived libcoda submodules:

```bash
nix develop
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

Required CI runs this Seamwork-native path with GCC and Clang through the checked-in flake.

The legacy aggregate and its former submodules are no longer part of the Seamwork checkout. For historical reproductions, use the original repositories and pinned revisions in the [migration record](docs/migration/seamwork-rebrand.md).

See [`docs/build.md`](docs/build.md) for the active build contract.

## Lineage

Seamwork preserves the Git history and lessons of `libcoda`.

The repository was renamed from `libcoda` to `seamwork` after the active C++23 build and study surface reflected the new purpose. Git history remains the canonical record of the earlier project.

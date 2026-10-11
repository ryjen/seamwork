# Seamwork charter

## Purpose

Seamwork is a practical laboratory for refining C++ interfaces and architecture with modern C++23.

The project exists to explore whether an interface or architectural boundary can be made easier to use, harder to misuse, and easier to reason about without hiding ownership, lifetime, allocation, blocking, failure, concurrency, or other important runtime consequences.

The reusable code is evidence of successful refinement, not the primary product.

## Origin

Seamwork began as `libcoda`, a collection of C++ utilities created while learning modern C++ and experimenting with interfaces that felt cleaner than the alternatives available at the time. Those libraries were used by real consumers, including a command-line Yahtsee application.

The re-charter keeps that original interest in interface quality while replacing the generic "utility library" goal with an explicit design and architecture laboratory.

## Goals

### Interface refinement

- Prefer APIs that communicate intent at the call site.
- Make invalid or ambiguous use difficult where the type system can help.
- Keep ownership, lifetime, cost, and failure semantics visible.
- Compare alternatives rather than assuming one C++ idiom is universally superior.

### Modern C++23

C++23 is the baseline for new and materially refined Seamwork code.

Use modern language and library facilities when they improve the design, including concepts, ranges, `std::expected`, `std::span`, `std::string_view`, `std::variant`, `std::format`, constexpr evaluation, constrained templates, and other C++23 facilities.

Modern syntax is not itself a goal. A simpler older mechanism remains preferable when it communicates the semantics better.

### Architecture

- Apply dependency inversion at meaningful capability boundaries.
- Keep pure/domain behavior independent from infrastructure when that improves testability and substitution.
- Prefer composition and value semantics over inheritance by default.
- Apply SOLID as design guidance, not as a class-count target.
- Avoid architectural ceremony in small examples.

### Practical evidence

Every substantial abstraction should be justified by at least one realistic consumer, study, test, benchmark, or security property.

Examples should exercise actual concerns such as parsing, error handling, persistence, randomness, I/O, formatting, concurrency, networking, or resource ownership rather than existing solely to demonstrate syntax.

### Engineering quality

- Deterministic tests for pure behavior.
- Integration tests at real boundaries.
- Static analysis and sanitizers where applicable.
- Fuzzing for parsers and untrusted-input surfaces.
- Benchmarks when a design changes meaningful runtime or compile-time cost.
- Security boundaries and unsafe assumptions documented explicitly.

## Non-goals

Seamwork is not:

- a general-purpose replacement for the C++ standard library;
- a replacement for mature projects such as Boost, fmt, Asio, or database clients merely for the sake of owning an implementation;
- a dumping ground for reusable snippets;
- a framework that imposes Clean Architecture on every program;
- an exercise in using every new C++ feature;
- committed to preserving legacy `libcoda` APIs when they obstruct a better study or design;
- necessarily a collection of independently released libraries.

Existing code may be retained as historical material, a practical consumer, or a starting point for refinement without being promoted as recommended modern C++.

## Design test

A new or promoted abstraction should answer three questions:

1. Is the interface meaningfully better for a real use case?
2. Are its ownership, lifetime, cost, and failure semantics still understandable?
3. Is there evidence that the abstraction should exist?

If those answers are unclear, keep the work as a study rather than a reusable component.

## Promotion model

```text
problem
  -> straightforward implementation
  -> identify friction / coupling / ambiguity
  -> compare alternatives
  -> refine with appropriate C++23 mechanisms
  -> test / analyze / benchmark
  -> document trade-offs
  -> optionally promote to a reusable component
```

Promotion is intentionally optional. A study that demonstrates why an abstraction should *not* exist is still a useful Seamwork result.

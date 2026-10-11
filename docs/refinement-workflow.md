# Refinement workflow

Seamwork preserves the reasoning that leads to an interface, not just the final implementation.

## Lifecycle

```text
Problem
  |
  v
Straightforward implementation
  |
  v
Identify awkward seams, coupling, ambiguity, and failure modes
  |
  v
Questions / Alternatives / Recommendations / Trade-offs
  |
  v
C++23 refinement
  |
  v
Tests + analysis + benchmark where relevant
  |
  v
Real consumer validation
  |
  +--> keep as study
  |
  +--> promote to reusable component
```

## Study structure

A study should normally capture:

### Problem

What concrete consumer or design problem exists?

### Baseline

Show the straightforward implementation before introducing abstractions. A baseline keeps the study honest and prevents comparing a refined design only against a straw man.

### Friction

Record what is actually difficult:

- ambiguous ownership;
- broad coupling;
- poor error handling;
- awkward call-site syntax;
- hard-to-test infrastructure;
- excess copying/allocation;
- unsafe defaults;
- unnecessary compile-time coupling;
- lifecycle ambiguity.

### QART

Use Questions, Alternatives, Recommendations, and Trade-offs before committing to a durable architecture decision.

Relevant alternatives may include:

- concrete composition;
- concepts/constrained templates;
- virtual interfaces;
- callables/policy objects;
- type erasure;
- variants;
- `std::expected` versus exceptions;
- owning values versus views/references.

### Evidence

Choose evidence according to the claim:

- unit or property tests for semantics;
- integration tests for boundaries;
- sanitizer/static-analysis results for safety;
- fuzzing for parsers and untrusted inputs;
- runtime benchmarks for performance claims;
- compile-time/binary-size measurements when template strategy is part of the decision;
- example application code for ergonomics.

### Decision

State what improved and what became worse.

A modern-looking implementation is not automatically the recommended result.

## Promotion criteria

A study may move into `components/` when:

- a real consumer needs it;
- its contract is clear;
- ownership/lifetime/error behavior is documented;
- relevant tests exist;
- dependencies are appropriate;
- the abstraction is narrower or clearer than simply using an existing mature library;
- normally, at least two real consumers need it, or a compelling independent contract/lifecycle justifies promotion with one.

Promotion is not required for a successful study.

## Example: dependency inversion

A study using the Yahtzee random source might compare:

```text
concrete PRNG dependency
    vs
virtual RandomSource
    vs
template <RandomSource R>
    vs
callable generator
    vs
type-erased random source
```

The objective is not to prove that concepts are better than virtual interfaces. The objective is to determine which seam best matches the required substitution, lifetime, ABI, testability, compile-time, and performance constraints.

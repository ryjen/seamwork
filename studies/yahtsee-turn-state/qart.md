# QART: typed Yahtsee turn states

## Questions

- How should a small closed state machine make invalid combinations difficult to represent?
- Does a `std::variant` of state-specific values communicate more than an enum plus flags?
- Would the classic virtual State pattern improve extensibility enough to justify ownership and dispatch complexity?
- Where should invalid commands be rejected?

## Alternatives

### Enum plus mutable flags

A single object with a phase enum, roll counter, hold mask, and permission flags is compact and familiar.

Its weakness is representational: the type can hold combinations that the domain says cannot exist. Correctness then depends on every mutation synchronizing multiple fields.

### One mutable class with guarded methods

Encapsulation can hide the flags and centralize checks. This improves local discipline but still stores a broad state shape and often turns methods into phase-dependent conditionals.

### Virtual State pattern

A base state interface with one derived class per phase makes behavior state-specific and supports runtime extension.

For this domain the state set is closed. Runtime polymorphism therefore adds allocation/ownership questions, indirection, and a broader interface without a demonstrated extension requirement.

### `std::variant` of value states

A variant makes the closed alternatives explicit and keeps ownership/value semantics straightforward. Each alternative carries only data meaningful to that state.

The cost is visitor/dispatch syntax and compile-time coupling to the closed state set. Adding a new state requires updating relevant visitors.

## Recommendation

Use a `std::variant` of state-specific values for this closed turn lifecycle.

Keep transition functions small and value-oriented. At the external command boundary, return a typed `std::expected` when a command is invalid for the current state.

This separates two ideas:

1. impossible *internal combinations* should be excluded by representation;
2. invalid *external commands* are legitimate input failures and need an explicit error result.

## Trade-offs

Benefits:

- contradictory phase/flag combinations disappear;
- state-specific roll-count enums exclude ordinary out-of-range lifecycle counts;
- state-specific data is visible in the type;
- no heap allocation or shared ownership is needed;
- transitions are deterministic value transformations;
- tests can assert exact state alternatives.

Costs:

- `std::visit` dispatch is more syntactically involved than a switch;
- the state set is intentionally closed;
- command handling still needs an error channel;
- excessive decomposition into tiny state types would become ceremony for simpler lifecycles.

The result is therefore a domain-specific state representation, not a recommendation to build a generic variant-based state-machine library.

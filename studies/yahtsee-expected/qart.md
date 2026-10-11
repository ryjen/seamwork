# QART: recoverable Yahtsee scoring errors

## Questions

- Should invalid CLI/domain input be represented by exceptions, `std::optional`, or `std::expected`?
- How much error detail does the caller need to recover or present a useful message?
- Once values are validated, should downstream scoring still carry an error channel?
- Does an explicit result type clarify the call site or merely add ceremony?

## Alternatives

### Exceptions

Exceptions keep the success return type simple and remain appropriate for failures that are exceptional at the current abstraction level. For malformed user input, rejection is expected behavior; requiring `try`/`catch` makes the ordinary recovery path less visible.

### `std::optional<T>`

`optional` is compact when the only useful distinction is value versus no value. Here the caller benefits from knowing why validation failed: unknown category, wrong number of dice, or a specific invalid die value.

### `std::expected<T, E>`

`expected` makes both success and recoverable failure part of the function contract and carries typed context without exceptional control flow. The cost is visible plumbing and making error types part of the API.

### Return `expected` everywhere

Making scoring return `expected<unsigned, E>` after the hand and category are already valid would invent a failure mode the operation does not have. An error channel should represent a real possibility, not architectural fashion.

## Recommendation

Use `std::expected` at the recoverable input/domain boundary:

- `parse_category(...)`;
- `make_hand(...)`.

After those operations establish invariants, use an ordinary total scoring function:

```cpp
score(category, hand) -> unsigned
```

This keeps failure explicit where it exists and absent where it does not.

## Trade-offs

Benefits:

- recoverable failure is visible in the signature;
- typed errors preserve useful context;
- no exception is needed for ordinary invalid input;
- validated domain values simplify downstream code.

Costs:

- error structures become part of the interface;
- explicit branching is more verbose than unchecked conversion;
- callers can still discard detail if they reduce errors too early.

The study therefore recommends `std::expected` selectively, not as a project-wide default return type.


## Consumer-driven follow-up: trusted Yahtsee hands (#46)

The composed Yahtsee consumer already has a typed randomness hand whose five faces are guaranteed to be in the domain range. To score it, the composition root currently converts those faces to five integers and calls `make_hand(...)`, which validates them again.

That looks redundant, so #46 compared ways to remove it.

### A. Keep conversion + validation

The current path is:

```text
randomness::hand
  -> array<int, 5>
  -> make_hand(span<const int>)
  -> scoring::hand
  -> score(...)
```

The cost is bounded and visible: five enum-to-integer conversions, one fixed-size count check, five range checks, and construction of a five-byte validated value array. No allocation is required on the success path.

The important semantic property is that the scoring study has **one construction boundary** for its validated hand. It does not need to trust another study's invariants.

### B. Public unchecked/trusted constructor

A `from_trusted_values(...)` or public constructor could skip validation for known-good callers.

Rejected for now. The scoring type could no longer enforce its own invariant through the public API; "trusted" would be a convention rather than a type property. The saved work is five range checks, which is not enough evidence to justify the weaker contract.

### C. Score `randomness::hand` directly

An overload such as:

```cpp
score(category, const seamwork::studies::yahtsee::hand&)
```

would remove conversion and validation.

Rejected. It would make the scoring study depend on the randomness study's concrete representation. That reverses the desired independence between study boundaries just to remove a tiny adapter.

### D. Promote a shared Yahtsee face/hand type

A shared domain value could let randomness and scoring use the same validated representation without coupling to either study.

Deferred to #47. One composed consumer has exposed the duplication, but Seamwork's promotion rule normally requires a second real consumer before creating a shared component. The second consumer should show whether the same domain contract actually recurs.

### E. Generic "face-like range" template

A constrained template could accept any fixed-size range whose elements can be converted to a face value.

Rejected. It broadens the API and shifts validation/diagnostics into a generic abstraction without a demonstrated need. The problem is domain identity, not container polymorphism.

## Decision for #46

Keep the explicit conversion and revalidation.

This is intentionally not a performance optimization target. There is no measured performance problem, so a benchmark is not justified.

The current boundary has three useful properties:

- untrusted/raw scoring input remains fail-closed;
- the scoring study owns and enforces its own invariant;
- cross-study coupling remains explicit at the composition root.

Revisit shared Yahtsee domain values only after the second practical consumer in #47 supplies additional evidence.

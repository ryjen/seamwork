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

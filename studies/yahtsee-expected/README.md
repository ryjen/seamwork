# Yahtsee `std::expected` study

Issue #32 examines recoverable failure handling at a small, real Yahtsee boundary.

The question is not whether `std::expected` is "more modern" than exceptions. The question is where explicit result types improve the contract without infecting already-valid domain logic with unnecessary error plumbing.

## Boundary

The study models two operations that can reasonably fail because of user or external input:

- parse a scoring category supplied as text;
- validate five raw integer die values.

Both return `std::expected<T, E>` with typed error information.

Once those operations succeed, the resulting category and hand satisfy the scoring preconditions. Scoring itself therefore returns an ordinary `unsigned` value.

```text
raw category text --------> expected<category, category_error> --+
                                                                  |
raw integer dice ---------> expected<hand, hand_error> -----------+--> score(...) -> unsigned
```

## Domain-value promotion

The validated five-face value is now owned by the promoted `seamwork::yahtsee` domain component (#51).

This study still owns the **error-handling question**: raw integer input is validated into that shared domain value with `std::expected`, while scoring a validated hand remains total.

The promotion removes duplicate hand representations; it does not remove the raw-input validation boundary.

## Scope

The scoring slice includes ones through sixes, chance, and Yahtsee. That is enough to exercise parsing, validation, and scoring contracts without turning the study into a complete game implementation.

## Selected design

Use `std::expected` when failure is an ordinary recoverable outcome, the caller is expected to branch on it, and useful structured error information exists.

Do not return `expected` from operations that are total once domain invariants have been established.

See [QART](qart.md) for alternatives and trade-offs.

## Build

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_yahtsee_expected
```

The example target is `seamwork_yahtsee_expected_example`.

## Promotion status

The `std::expected` **error-handling study remains study-only**.

Only the stable Yahtsee face/hand domain value was promoted. Seamwork still has no project-wide result/error framework.

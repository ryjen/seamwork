# Yahtsee turn-state study

Issue #38 examines how to represent a small, closed domain state machine without accumulating correlated mutable flags.

## Baseline

A straightforward mutable design often grows toward something like:

```cpp
struct turn {
    phase current_phase;
    unsigned rolls_taken;
    hold_mask held;
    bool may_hold;
    bool must_score;
};
```

That representation can express contradictory combinations such as `may_hold == true` while awaiting an initial roll, or `must_score == false` after the third roll. Every mutator must preserve all of those relationships.

## Selected design

The refined model uses state-specific value types:

```text
awaiting_roll
choosing_dice
must_score
completed
```

and stores exactly one of them in `std::variant`.

Data belongs to the states where it has meaning. Roll counts are encoded with state-specific enum types, so a normal caller cannot construct a choice state with zero or three completed rolls. The forced-score state cannot represent a reroll permission flag at all.

The command/application boundary still accepts commands that may be invalid for the current state, so `apply(...)` returns `std::expected<turn_state, transition_error>`. Internal state representation and external command validity are separate concerns.

## Turn lifecycle

```text
awaiting roll
  -> roll completed
choosing dice
  -> toggle holds
  -> reroll -> awaiting roll
  -> score  -> completed

after third roll
  -> must score
  -> score -> completed
```

## Why `std::variant` here?

The state set is small and closed, the states are values, and there is no plugin/ABI extension requirement. Runtime inheritance would add object lifetime and dispatch concerns without buying a useful extension seam.

See [QART](qart.md) for the alternatives and trade-offs.

## Build

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_yahtsee_turn_state
```

## Promotion status

**Study only.**

This is not a generic state-machine framework. The domain-specific representation is the point.

# Composed Yahtzee turn example

Issue #40 is the first maintained consumer that forces multiple Seamwork studies to collaborate.

It composes:

- the [randomness seam](../../studies/yahtzee-randomness/README.md);
- the [`std::expected` scoring boundary](../../studies/yahtzee-expected/README.md);
- the [typed turn-state model](../../studies/yahtzee-turn-state/README.md).

## Scripted scenario

The deterministic example:

1. rolls `1 2 3 4 5`;
2. holds the second and fifth dice;
3. rerolls the remaining dice as `6 6 6`;
4. produces `6 2 6 6 5`;
5. parses the `sixes` category;
6. scores `18`;
7. completes the turn after two rolls.

## Integration findings

### Duplicate hold state

The randomness study's `hand` owns a hold mask because it needs to know which dice to reroll.

The turn-state study also owns a hold mask because holding is part of the legal turn lifecycle.

The composed example must currently update both explicitly. That is genuine duplication and a possible future refinement target, but one consumer is not enough evidence to extract a shared component yet.

### Duplicate hand representations

The randomness study models faces as a typed `face` enum. The scoring-boundary study intentionally accepts raw integers at its validation boundary and produces its own validated `hand`.

The example therefore performs a small explicit conversion before scoring.

This is cheap and understandable. It is recorded as evidence rather than hidden behind a generic adapter framework.

### Composition root remains simple

No service locator, factory hierarchy, dependency-injection container, or application framework is needed. The application directly composes value-oriented study APIs and handles their explicit error results.

## Build

From the repository root:

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_example_yahtzee_turn
./build/dev/examples/yahtzee-turn/seamwork_example_yahtzee_turn
```

## Promotion status

This example does **not** promote any study into `components/`.

Its purpose is to create consumer evidence for future refinement decisions.

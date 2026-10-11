# Yahtsee scorecard example

Issue #47 is Seamwork's second maintained Yahtsee consumer.

It deliberately does not wrap or depend on `examples/yahtsee-turn`. Instead, it composes the study interfaces independently in a different workflow: two turns, two score categories, and score accumulation.

## Scripted scenario

The deterministic scorecard plays:

1. a Yahtsee turn:
   - initial dice: `6 6 2 3 4`;
   - hold the first two sixes;
   - reroll the remaining dice as `6 6 6`;
   - score `yahtsee` for 50;
2. a Chance turn:
   - roll `1 2 3 4 5`;
   - score immediately for 15;
3. accumulate a total of 65.

## Consumer evidence

### Repeated adapter produced promotion evidence

The first version of this consumer independently repeated the same adapter already present in `examples/yahtsee-turn`:

```text
randomness::hand
  -> five integers
  -> scoring::make_hand(...)
  -> scoring::hand
```

That repetition was the evidence required by #51. The shared Yahtsee face/hand contract is now promoted, so this consumer passes the typed hand directly from randomness to scoring.

The raw scoring-input path still validates separately; the promotion removed duplicated representation rather than weakening the boundary.

### Turn-state ownership still composes cleanly

Hold decisions remain owned by the turn-state model. The randomness operation borrows the fixed-size hold view only when rerolling. The second consumer does not need a new adapter for that seam.

### No application framework emerged

A couple of local helper functions are enough to avoid repetitive error plumbing inside this example. No service locator, factory hierarchy, DI container, or generic state-machine framework is needed.

## Promotion result

The two maintained consumers justified exactly one shared component:

- a **Yahtsee-specific six-face/five-value domain contract**.

Still not justified:

- generic dice/hand component;
- generic scorecard framework;
- generic application/composition framework.

The promoted contract is intentionally smaller than the studies that depend on it.

## Build

From the repository root:

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_example_yahtsee_scorecard
./build/dev/examples/yahtsee-scorecard/seamwork_example_yahtsee_scorecard
```

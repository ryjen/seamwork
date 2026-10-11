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

### The scoring adapter repeats

This consumer independently needs the same boundary seen in `examples/yahtsee-turn`:

```text
randomness::hand
  -> five integers
  -> scoring::make_hand(...)
  -> scoring::hand
```

#46 deliberately retained that boundary because bypassing validation would either weaken the scoring invariant or couple the scoring study directly to the randomness study.

Now that the conversion has appeared in **two maintained consumers**, a shared domain value has stronger evidence than it had after the first example. That does not automatically justify promotion, but it is now a legitimate candidate for a focused follow-up decision.

### Turn-state ownership still composes cleanly

Hold decisions remain owned by the turn-state model. The randomness operation borrows the fixed-size hold view only when rerolling. The second consumer does not need a new adapter for that seam.

### No application framework emerged

A couple of local helper functions are enough to avoid repetitive error plumbing inside this example. No service locator, factory hierarchy, DI container, or generic state-machine framework is needed.

## Promotion evidence

Candidate worth evaluating next:

- a **Yahtsee-specific five-face validated domain value** shared by randomness/scoring consumers.

Not yet justified:

- generic dice/hand component;
- generic scorecard framework;
- generic application/composition framework.

The follow-up should compare a small shared Yahtsee domain contract against leaving the duplicated conversion in place. Promotion is still optional.

## Build

From the repository root:

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_example_yahtsee_scorecard
./build/dev/examples/yahtsee-scorecard/seamwork_example_yahtsee_scorecard
```

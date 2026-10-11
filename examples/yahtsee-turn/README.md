# Composed Yahtsee turn example

Issue #40 is the first maintained consumer that forces multiple Seamwork studies to collaborate.

It composes:

- the [randomness seam](../../studies/yahtsee-randomness/README.md);
- the [`std::expected` scoring boundary](../../studies/yahtsee-expected/README.md);
- the [typed turn-state model](../../studies/yahtsee-turn-state/README.md).

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

### Hold ownership refinement

The first version of this example demonstrated duplicate hold state: both the randomness `hand` and the turn-state model stored the same decisions.

Issue #42 uses that consumer evidence to refine the seam without extracting a shared component. Turn state is now the single authority for held dice. The randomness operation accepts a borrowed fixed-extent hold view only while rerolling.

This removes synchronization work while keeping the studies independent: the randomness study knows only `std::span<const bool, 5>`, not the turn-state implementation type.

### Scoring boundary evolution

This example originally converted the randomness hand back to raw integers and revalidated it through the scoring boundary.

#46 deliberately kept that adapter after one consumer because removing it would have weakened validation or promoted a shared dependency without enough evidence.

#47/#50 then added a second independent consumer and reproduced the same adapter. That additional evidence justified #51's narrow promotion of the Yahtsee face/hand value contract.

The example now passes the shared typed hand directly into scoring. Raw external input still validates through the shared Yahtsee factory; no unchecked scoring path was introduced.

### Composition root remains simple

No service locator, factory hierarchy, dependency-injection container, or application framework is needed. The application directly composes value-oriented study APIs and handles their explicit error results.

## Build

From the repository root:

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_example_yahtsee_turn
./build/dev/examples/yahtsee-turn/seamwork_example_yahtsee_turn
```

## Promotion result

This example, together with the independent scorecard consumer, supplied the evidence for the narrow [Yahtsee domain component](../../components/yahtsee-domain/README.md).

No application, scoring, randomness, or turn-state framework was promoted.

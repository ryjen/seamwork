# Yahtsee randomness study

Issue #22 is Seamwork's first end-to-end refinement study.

It takes one useful seam from the historical C++ Yahtsee/libcoda code and rebuilds it deliberately in C++23 rather than porting the old utility library.

## Historical baseline

The old design had useful intent: inject a random engine so game tests could control rolls.

Its shape was roughly:

```text
player
  -> dice
      -> die
          -> raw die::engine*
              -> virtual generate(from, to)
```

The implementation also carried several unrelated properties:

- each die retained a raw engine pointer with implicit lifetime;
- the default engine was global/static;
- invalid construction used `throw new invalid_argument`;
- `die::roll()` asserted that the engine pointer was valid;
- a mutable `sides(value)` setter could bypass the constructor invariant;
- Yahtsee-specific code lived inside the nominally generic dice library.

Those are baseline observations, not requirements to preserve.

## Scope

This study keeps only the behavior needed to exercise the seam:

- five six-sided values form a valid Yahtsee hand;
- a first roll produces all five values;
- a reroll receives a caller-owned fixed-size hold selection;
- a reroll asks the source only for unheld dice;
- production randomness is backed by standard C++ random facilities;
- tests can provide exact semantic face sequences.

Scoring, networking, terminal UI, persistence, and the broad generic dice API are intentionally out of scope.

## Selected design

The primary variant uses:

- a small `face` domain enum;
- a value-semantic `hand` that owns only rolled face values;
- a fixed-extent borrowed `hold_view` for reroll policy;
- a C++20/23 concept describing a semantic `face_source`;
- caller-owned source lifetime through a reference;
- a `uniform_face_source` adapter around any standard URBG.

The study also compiles runtime-interface, type-erased, and direct-URBG alternatives so the recommendation is based on concrete shapes rather than a straw-man comparison.

See [QART](qart.md) for the decision.

## Consumer-driven refinement

The first composed Yahtsee example exposed duplicate ownership: both the randomness study and the turn-state study stored which dice were held. Issue #42 removes that duplication.

The randomness study now owns only dice values. Reroll policy is supplied explicitly as a borrowed fixed-extent view:

```cpp
reroll(current_hand, hold_view, source)
```

In the composed consumer, the turn-state model remains the single authority for hold decisions and the randomness seam borrows that state only for the duration of the reroll call. This keeps ownership and lifetime visible without coupling the randomness study to the concrete turn-state type.

## Build

From the repository root:

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_yahtsee_randomness
```

The example target is `seamwork_yahtsee_randomness_example`.

## Promotion status

**Study only.**

Nothing here is yet a general Seamwork component. The point is to validate the refinement workflow and a real consumer-facing seam first.

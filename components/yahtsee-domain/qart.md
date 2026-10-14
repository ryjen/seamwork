# QART: promote a shared Yahtsee five-face domain value?

## Question

After two maintained consumers repeat the same randomness-to-scoring adapter, has a shared Yahtsee domain value earned promotion?

## Evidence

Both consumers independently performed:

```text
randomness::hand
  -> five integers
  -> scoring::make_hand(...)
  -> scoring::hand
```

The conversion is cheap, and #46 correctly kept it after one consumer. The important new evidence is **repetition across independent consumers**, not runtime cost.

## Alternatives

### A. Keep local adapters

This preserves maximal study independence.

It remains viable, but the same stable domain facts—six face values and exactly five dice—would continue to be represented twice and converted at every composition root.

### B. Share only `face`

This removes one duplicated type but leaves separate hand representations and the adapter shape largely intact.

The consumers demonstrate the whole fixed five-face value as the recurring contract, so face-only promotion does not address the actual seam.

### C. Promote Yahtsee `face` + `hand`

Selected.

The promoted component contains only domain values and raw validation. Randomness and scoring depend on it but remain behaviorally separate.

This removes the repeated conversion without inventing a cross-study dependency.

### D. Generic dice component

Rejected.

No evidence supports arbitrary side counts, generic dice collections, game policies, or reusable dice frameworks. That would repeat the historical utility-library drift.

## Validation ownership

Raw/user input still goes through:

```cpp
seamwork::yahtsee::make_hand(std::span<const int>)
    -> std::expected<hand, hand_error>
```

The validated `hand` then flows directly into scoring.

Random generation produces typed `face` values and constructs a typed `hand` directly.
The public constructor independently validates the enum representations,
so even a malformed custom face source cannot bypass the hand invariant.

This creates one invariant owner rather than an unchecked bypass.

## Hold ownership

Hold state remains outside the component.

#42 already established the turn-state model as the single authority for hold decisions. A shared hand therefore contains only face values.

## Recommendation

Promote the Yahtsee-specific domain value.

Do not promote a generic dice library.

The promotion is justified by repeated consumer evidence and by cleaner dependency direction, not by code deduplication alone.

# Yahtsee domain component

This is Seamwork's first promoted component.

It exists because two independent maintained consumers demonstrated the same stable domain contract and the same adapter friction:

- `examples/yahtsee-turn`;
- `examples/yahtsee-scorecard`.

Both needed exactly five valid Yahtsee faces. Before promotion, randomness and scoring each owned a separate hand representation and both consumers converted between them.

## Contract

The component owns only stable domain facts:

- a six-value `face` enum;
- exactly five faces per `hand`;
- raw integer validation into a typed hand;
- typed validation errors.

It deliberately does **not** own:

- random generation;
- hold state;
- turn lifecycle;
- scoring rules;
- scorecards;
- persistence/UI/networking;
- generic arbitrary-sided dice.

## Dependency direction

```text
             seamwork::yahtsee
             face + hand + validation
                ^          ^
                |          |
        randomness       scoring
                ^          ^
                 \        /
                  consumers
```

The randomness and scoring studies depend inward on the domain value. They do not depend on each other.

## Why promotion is justified

After the first consumer, #46 intentionally kept the explicit adapter because one use site was not enough reason to create a shared dependency.

#47/#50 added a second independent consumer and reproduced the same conversion. That changed the evidence: the type is no longer merely convenient for one example.

See [QART](qart.md) for the promotion decision.

## Non-goal

This is not the resurrection of `libcoda-dice`.

If another game/domain later needs dice, it should provide separate evidence before any generic dice abstraction is considered.

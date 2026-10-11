# QART — randomness seam for the Yahtsee reference study

## Question

What abstraction, if any, should the Yahtsee domain depend on to obtain random die faces?

The historical implementation answered this with a nested virtual `die::engine` stored as a raw pointer by every die. That enabled deterministic tests, but it also mixed random generation with die state, left ownership implicit, required virtual dispatch for a very small capability, and made the generic dice abstraction broader than the game actually needed.

The Seamwork study narrows the requirement to the semantic operation the domain needs:

> produce the next valid six-sided Yahtsee face.

## Constraints

- Five dice per hand.
- Held dice are not rerolled, but hold decisions belong to the caller/turn lifecycle rather than the random-value object.
- Tests must be able to provide an exact deterministic sequence.
- The domain should not own or hide the lifetime of the random source.
- A normal game should be able to use a standard C++ random engine.
- The seam should not imply that Seamwork needs a general-purpose dice library.
- Runtime plugin/substitution support is not currently a requirement.

## Alternatives

### A. Semantic callable constrained by a concept

```cpp
template <class Source>
concept face_source = requires(Source& source) {
    { std::invoke(source) } -> std::same_as<face>;
};
```

The domain operation takes `Source&`. The caller owns its lifetime. A production adapter maps a standard URBG through `std::uniform_int_distribution`; a test can provide a tiny sequence callable.

**Advantages**

- expresses the domain capability directly;
- no ownership ambiguity;
- no required allocation or virtual dispatch;
- deterministic fakes are trivial;
- source type can retain state naturally;
- invalid numeric ranges stay outside the domain operation.

**Costs**

- operation is templated and implemented in a header;
- source type participates in compilation;
- not a stable runtime ABI seam.

### B. Narrow virtual interface

```cpp
struct virtual_face_source {
    virtual ~virtual_face_source() = default;
    virtual face next() = 0;
};
```

**Advantages**

- stable runtime substitution point;
- implementation type can remain out of consumer headers;
- conventional dependency inversion.

**Costs**

- object/lifetime management remains a concern;
- virtual dispatch is unnecessary for the current consumer;
- mocking is heavier than a callable;
- creates a named runtime abstraction before runtime variation is needed.

This is substantially cleaner than the historical raw engine pointer, but still stronger machinery than the current problem requires.

### C. Type-erased callable

```cpp
using erased_face_source = std::function<face()>;
```

**Advantages**

- runtime substitution without an inheritance hierarchy;
- compact consumer-facing type;
- easy adaptation from lambdas and stateful objects.

**Costs**

- type erasure may allocate;
- copy semantics and ownership of captured state require attention;
- hides concrete source characteristics that are visible with a template parameter.

This becomes attractive at a runtime composition boundary, but there is no such boundary in the first example.

### D. Depend directly on `std::uniform_random_bit_generator`

The domain can accept a standard URBG and construct `std::uniform_int_distribution` internally.

**Advantages**

- no project-specific source abstraction;
- uses a standard C++ concept;
- makes the random engine explicit.

**Costs**

- the domain now knows that its dependency is a random-bit engine rather than simply a face producer;
- deterministic test doubles must satisfy the URBG contract;
- distribution/mapping policy becomes part of the domain operation;
- a test that wants the exact sequence `{4, 3, 6, 1, 3}` has to reason below the semantic level being tested.

## Recommendation

Use **A: a semantic callable constrained by `face_source`** for this study.

It is the narrowest seam that satisfies the real requirements. It preserves caller-owned lifetime and deterministic testing without introducing a runtime abstraction that the application does not currently need.

The production implementation remains standard-library based:

```text
std::mt19937
    -> std::uniform_int_distribution<1, 6>
    -> uniform_face_source::operator()
    -> face
    -> initial_roll / reroll
```

If a later application requires runtime-selected random providers or a stable ABI boundary, revisit type erasure or a runtime interface at that composition boundary rather than designing for it pre-emptively.

## Trade-off accepted

The selected `initial_roll` and `reroll` operations are templates. For this tiny domain seam, that compile-time coupling is accepted in exchange for direct semantics and trivial deterministic sources.

A later composed consumer (#40/#41) showed that storing the hold mask inside the randomness `hand` duplicated state already owned by the turn lifecycle. The refined `reroll` therefore borrows `std::span<const bool, 5>` instead. The fixed extent keeps the five-die contract explicit; borrowing keeps lifetime/cost visible; and the randomness study does not depend on the turn-state implementation type.

A benchmark is not currently justified: none of the alternatives is being selected on a performance claim. If runtime or compile-time cost becomes a reason to change the design, add measurement before making that argument.

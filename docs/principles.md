# Seamwork design principles

These principles guide new code and material refactors. They are review questions, not mechanical rules.

## 1. Keep consequences visible

A convenient interface must not make important behavior surprising.

Callers should be able to reason about:

- ownership and borrowing;
- object lifetime;
- allocation and copying;
- blocking and I/O;
- synchronization and thread-safety;
- failure and cancellation;
- resource acquisition and release.

Ergonomics should reduce incidental complexity without erasing operational semantics.

## 2. Prefer values and explicit ownership

Prefer value semantics when values correctly model the domain.

Use references, views, smart pointers, handles, or explicit owner types according to the actual lifetime relationship. Avoid `shared_ptr` as a default dependency-injection mechanism and avoid raw owning pointers.

## 3. Use the narrowest useful seam

Introduce an abstraction when there is a real axis of variation, test boundary, policy boundary, or resource boundary.

A seam may be represented by:

- a concrete value type;
- a callable;
- a template parameter;
- a concept;
- a small runtime interface;
- type erasure;
- a variant;
- a composition root.

Do not assume virtual inheritance is the default expression of dependency inversion.

## 4. Apply SOLID pragmatically

- **SRP:** separate concerns that have materially different reasons to change.
- **OCP:** make known variation extensible without scattering conditionals through stable policy.
- **LSP:** interchangeable implementations must preserve documented behavior, not merely a base-class shape.
- **ISP:** expose the capabilities a caller needs rather than broad service objects.
- **DIP:** policy should depend on stable abstractions or capabilities, while concrete infrastructure remains at the edge.

SOLID does not require deep hierarchies, factories, or an interface for every class.

## 5. Standard library first

Prefer C++23 standard facilities when they express the required semantics well.

Project-specific wrappers need a concrete justification such as:

- stronger domain invariants;
- clearer ownership or error semantics;
- portability around an unstable platform boundary;
- a deliberately narrower capability;
- measurable ergonomics or safety improvement.

Do not wrap a standard type solely to make the API look project-specific.

## 6. Make failure part of the interface

Expected operational failure should be represented deliberately.

Choose among values, `std::expected`, optionality, exceptions, or domain error types based on the caller's recovery model. Do not use assertions, null dereferences, or process termination as normal library error handling.

Avoid mixing incompatible failure models inside one abstraction without a documented reason.

## 7. Separate policy from infrastructure when useful

Pure rules and transformations should generally remain testable without live infrastructure.

Database, network, filesystem, clock, randomness, and terminal boundaries are useful seams when the application actually needs substitution, determinism, or policy separation.

Do not introduce ports/adapters merely to satisfy an architectural diagram.

## 8. Prefer compile-time abstraction only when it earns its cost

Concepts and templates can produce expressive, zero-overhead APIs, but they also affect build time, diagnostics, binary shape, ABI boundaries, and implementation exposure.

Compare compile-time polymorphism with runtime interfaces, type erasure, and plain composition when the trade-off is non-trivial.

## 9. Measure claims about cost

If a refinement is justified by runtime performance, allocation behavior, binary size, or compile time, add evidence appropriate to the claim.

"Zero cost" is not a substitute for measurement.

## 10. Treat examples as consumers

Examples should be maintained as real code:

- compile in CI;
- use supported interfaces;
- contain tests where appropriate;
- expose awkward APIs and architecture debt;
- provide feedback before a study is promoted into `components/`.

A focused C++23 Yahtsee slice is the initial reference consumer because it exercises domain rules, randomness, input/output, and test seams without carrying forward the historical application's unrelated networking/UI stack.

## 11. Security is an interface property

Untrusted input, credentials, network data, SQL, parsing, filesystem paths, and resource limits are part of API design.

Secure defaults, bounded resource behavior, validation boundaries, and redaction rules should be visible in the contract rather than left to individual callers.

## 12. Refinement over reinvention

The objective is not to own more code.

Use an existing mature library when it already provides the right contract. Seamwork is most useful where comparing or refining an interface teaches something transferable about modern C++ design.

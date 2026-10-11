# ADR 0002 — Transition toward a single Seamwork repository

## Status

Accepted and implemented for the primary repository by PR #35 under #34; independent historical repositories remain for provenance.

This ADR supersedes ADR 0001 as the target topology decision. ADR 0001 remains useful historical context for why the `libcoda` modernization originally retained component repositories.

## Context

`libcoda` was organized as an aggregate toolkit with independently versioned utility repositories connected through Git submodules. That topology matched the old project identity: format, database, network, dice, and other utilities were treated as potentially reusable libraries.

Seamwork has a different purpose. It is a C++23 interface and architecture refinement laboratory in which studies, examples, benchmarks, and selectively promoted components evolve together.

Under that model, repository boundaries should represent real independent lifecycle/release requirements rather than historical utility categories.

## Decision

Seamwork will move toward a single primary repository.

The existing submodules were retained during inventory/classification. With their dispositions and immutable pins recorded, the active Seamwork repository retires those gitlinks. Historical code remains available in prior commits and independent repositories.

A legacy component should remain an independent repository only if there is concrete evidence that it needs an independent:

- consumer base;
- release/version lifecycle;
- ownership boundary;
- dependency/toolchain boundary; or
- CI/security lifecycle.

Otherwise useful material should be absorbed into the appropriate Seamwork study, example, or component while preserving provenance.

## Why now

ADR 0001 deliberately deferred this decision until component contracts, tests, and modernization behavior were better understood.

That work has produced:

- target-local CMake ownership across major components;
- deterministic component test layers;
- sanitizer/fuzz/coverage patterns;
- explicit architecture and security boundaries.

More importantly, the project itself has now been re-chartered. The old repository boundaries describe the previous product model more than the new one.

## Transition strategy

1. Do not flatten submodules immediately.
2. Inventory each legacy repository using KEEP/REWORK/ABSORB/ARCHIVE/REMOVE.
3. Select the first practical consumer and refinement study.
4. Import only the code needed for active studies while preserving license/history attribution.
5. Remove a submodule after its useful material has been migrated or explicitly archived.
6. Reassess independent repositories only when a promoted component develops a real lifecycle need.

## Alternatives

### Keep the existing multi-repository aggregate permanently

This preserves independent histories and minimizes source-control changes, but it encourages Seamwork to remain organized around legacy utility-library categories and makes cross-cutting design studies unnecessarily expensive.

### Package-only composition

This creates strong release boundaries but assumes independently useful libraries. That is not the default Seamwork goal.

### Immediate monorepo flattening

This provides atomic changes quickly but would force disposition decisions before the legacy code has been reviewed. The incremental transition keeps source-control migration subordinate to design work.

## Consequences

Positive:

- studies, examples, benchmarks, and components can change atomically;
- repository organization follows the new project purpose;
- generic legacy utilities do not automatically become permanent Seamwork products;
- shared C++23 tooling and policies become simpler.

Negative:

- migration requires deliberate history/provenance handling;
- existing consumers may need redirects or compatibility releases;
- component repositories must be archived or maintained during transition;
- some independent components may eventually need to be split out again.

## Revisit criteria

Split a component back into an independent repository when actual operational evidence shows that independent lifecycle management is beneficial. Repository boundaries should follow real ownership and release needs rather than predicted reuse.

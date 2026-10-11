# Seamwork re-charter and migration

## Status

Core re-charter complete. PR #35 implements the primary repository's legacy build and submodule cutover under #34; GitHub archive settings for independent historical repositories are tracked separately.

This document tracks the transition from `libcoda` to Seamwork. The transition intentionally separates project purpose, code disposition, C++23 migration, and repository renaming so history is preserved and legacy code is not promoted accidentally.

## Target identity

**Seamwork — modern C++23 through interface and architecture refinement.**

The repository is becoming a design laboratory with practical consumers, studies, and selectively promoted components rather than an aggregate general-purpose utility toolkit.

## Migration principles

1. Preserve Git history and provenance.
2. Do not mechanically rename every `libcoda` component.
3. Review legacy code for educational or practical value before migration.
4. C++23 is required for new and materially refined Seamwork code.
5. Legacy C++17 code may remain temporarily while being classified or used as a baseline.
6. Prefer one coherent Seamwork repository unless a component demonstrates a real independent release/lifecycle need.
7. Keep compatibility shims only when they reduce migration risk; do not let compatibility define the new architecture.

## Proposed target layout

```text
seamwork/
├── docs/
│   ├── charter.md
│   ├── principles.md
│   ├── refinement-workflow.md
│   ├── design-notes/
│   ├── qart/
│   └── adr/
├── studies/
│   ├── interfaces/
│   ├── ownership/
│   ├── errors/
│   ├── polymorphism/
│   └── dependency-inversion/
├── components/
├── examples/
│   └── yahtzee/
├── benchmarks/
└── tests/
```

The layout is a target direction, not justification for empty directory scaffolding.

## Legacy disposition model

Each legacy area receives one disposition:

| Disposition | Meaning |
| --- | --- |
| KEEP | Useful practical consumer or component with a clear role in Seamwork. |
| REWORK | The problem remains useful but the implementation should become a C++23 refinement study. |
| ABSORB | Useful material should move into another study/component rather than remain independently branded. |
| ARCHIVE | Preserve as historical/reference material but do not present as recommended modern design. |
| REMOVE | No meaningful current purpose beyond accidental utility accumulation. |

## Final legacy disposition

The first disposition review is complete. These decisions determine what may receive further Seamwork modernization effort; they do **not** require immediate deletion of historical code.

| Legacy area | Final disposition | Rationale / preserved evidence | Follow-up |
| --- | --- | --- | --- |
| Yahtzee | **REWORK** (preserve lineage) | It is the strongest real historical consumer and a compact domain for interface studies. The old C++ application remains baseline evidence rather than a port target. | Continue focused C++23 studies; PR #23 established the randomness seam. |
| `libcoda-dice` | **ABSORB** | Generic dice and Yahtzee-specific behavior were already mixed. The useful randomness/domain problem belongs with the Yahtzee study, not an independent library. | Preserve history; migrate only behavior required by studies; retire the independent library when references are removed. |
| `libcoda-format` | **ARCHIVE** | C++23 `std::format` removes the main product justification. Its deterministic parser tests, fuzzing, sanitizer work, and bounded-input lessons remain valuable evidence. | Keep security/testing artifacts as reference; do not promote another formatter unless a study identifies a gap in standard/mature facilities. |
| `libcoda-db` | **ARCHIVE** | Adapter, transaction, URI, error, and backend-contract work is useful architectural source material, but Seamwork has no current consumer justifying a general DB abstraction. | Mine focused studies only when a real consumer needs them; preserve threat model and deterministic backend-test lessons. |
| `libcoda-net` | **ARCHIVE** | Transport/TLS/protocol boundaries are rich study material, but a broad networking library carries a large security and maintenance surface without a current Seamwork consumer. | Preserve threat model/test-boundary evidence; create future narrow studies from concrete consumers rather than maintain the library. |
| log | **ARCHIVE** | Historical variadic logging predates modern formatting/printing practice and has no current consumer justification. | Retain lineage only; use established logging facilities in examples unless logging itself becomes the study. |
| math / `bigint` | **ARCHIVE** | Arbitrary-precision arithmetic is a substantial specialist domain and not part of Seamwork's current interface-refinement thesis without a consumer. | Preserve implementation history; prefer a mature bigint implementation for future consumers. |
| string / argument / buffer helpers | **ARCHIVE** | The area combines tokenization, parsing, buffers, and generic string utilities. Much is better served by standard/mature facilities and no current consumer requires the old API. | Extract a focused parsing/interface study only if a real CLI example needs it. |
| terminal | **ARCHIVE** | VT100, cursor, color, output, and progress handling are infrastructure rather than a current design target. | Revisit as an adapter only for a practical CLI study; otherwise use a mature terminal library. |
| thread / `later` | **ARCHIVE** | The detached-thread helper has weak lifetime/cancellation semantics and predates `std::jthread`/stop-token-era design. It is useful as a negative baseline, not as a component. | Potential future structured-concurrency/lifetime study only when a consumer requires delayed work. |
| utility / collections | **REMOVE — completed in #26** | Generic collection helpers are precisely the utility-bucket model Seamwork is leaving behind and are largely superseded by ranges/standard algorithms. | Removed from the active tree; Git history preserves provenance. |
| json wrapper | **ARCHIVE** | C++23 has no standard JSON facility, but Seamwork has no reason to own a generic wrapper around json-c without a consumer. | Use a mature JSON library or narrow adapter in future examples; retain old code as history. |
| custom variant | **REMOVE — completed in #26** | The home-grown discriminated union is directly superseded by `std::variant` and has no remaining design role. | Removed from the active tree; retain only historical provenance. |
| shared `cmake` repo | **ABSORB selectively** | Build helpers are migration infrastructure, not a product. Useful helpers should live with the project or an organization-wide build standard. | Copy only still-needed helpers into the appropriate maintained location, then retire the independent dependency when unused. |

### Decision rule going forward

**ARCHIVE** does not mean "modernize before archiving." It means stop investing in the component as a product while preserving enough source, documentation, tests, threat models, and Git history to support future studies.

**REMOVE** means remove the code from the active Seamwork build/tree once references are eliminated; history remains available in Git.

**ABSORB** means migrate only the useful behavior/evidence into a concrete study or maintained build surface before retiring the old boundary.

No legacy component should receive a broad C++23 conversion simply because it exists.

## Inventory evidence

The first pass found several useful constraints:

- `ryjen/yahtsee` is archived and its default branch is Go.
- The recoverable `original` branch is C++14 and depends directly on old logging, string, dice, network, HTTP, async, terminal/libcaca, archive, and UPnP surfaces.
- The original Yahtzee test tree is effectively empty, so existing behavior must be characterized rather than assumed.
- `libcoda-dice` already mixes generic dice behavior with a `yaht` subtree, reinforcing that the historical library boundary was not especially strong.
- `libcoda-format` has some of the strongest existing modernization evidence (deterministic tests, fuzzing, target-local coverage), which is worth preserving even if the formatter itself is not promoted.
- DB and network components have useful adapter/security/test boundaries, but their scope is much broader than needed for an initial Seamwork example.

The first concrete study is tracked in #22: rebuild the smallest useful Yahtzee dice/randomness seam in C++23 and compare dependency-inversion mechanisms against a real consumer.

## C++23 migration

The current build is Seamwork-native: C++23 studies/tests build without archived libcoda submodules. The old `SEAMWORK_BUILD_LEGACY` compatibility path was retired after documenting the exact submodule pins. Historical reproduction uses the prior aggregate commit and component repositories.

Active Seamwork targets own their compile-feature declarations. Archived targets retain their historical contracts in their original repositories and the aggregate's Git history.

When a legacy component becomes an active Seamwork study/component:

1. establish a C++23 compiler/toolchain baseline;
2. characterize current behavior with tests;
3. identify the design question being studied;
4. compare standard-library/mature-library alternatives;
5. refactor only the parts required by the study;
6. update namespaces/targets only when the new contract is ready.

Do not perform a syntax-only "C++23 conversion."

## Rename sequence

1. Land charter, principles, workflow, and migration plan.
2. Establish C++23 at the aggregate/new-code boundary.
3. Inventory legacy components and decide KEEP/REWORK/ABSORB/ARCHIVE/REMOVE.
4. Build the first reference study/consumer under the new structure.
5. Make the Seamwork C++23 surface the default build/CI path and move archived libcoda behind explicit compatibility controls (#28).
6. Introduce `seamwork::`, `Seamwork::`, and `SEAMWORK_*` names for active new surfaces.
7. Provide temporary compatibility aliases where justified.
8. Rename the GitHub repository from `libcoda` to `seamwork`. **Completed.**
9. Update badges, package metadata, links, namespaces, include paths, and related repositories.
10. Archive or redirect legacy component repositories according to their disposition. **README redirects in place; GitHub archive settings pending.**

The repository rename occurred only after the charter, C++23 study surface, legacy disposition, and default-build inversion were established, so the new name does not imply that all historical libcoda code is endorsed as Seamwork.

## First milestone: Seamwork 0.1

- [x] Define the Seamwork charter.
- [x] Define design principles.
- [x] Define the refinement workflow.
- [x] Record the migration/disposition model.
- [x] Establish and validate the root C++23 build baseline.
- [x] Complete legacy component disposition review (#24).
- [x] Reconcile the existing multi-repository ADR with the new target topology.
- [x] Rebuild a focused Yahtzee slice as the first practical reference application (#22).
- [x] Complete one end-to-end refinement study.
- [x] Introduce the first intentional `seamwork::` study surface (#23).
- [x] Make Seamwork the default build/CI path and legacy libcoda explicitly opt-in (#28/#29).
- [x] Rename the GitHub repository to `ryjen/seamwork` after the active surface reflects the new identity.

## Legacy graph retirement and reproducible source pins (#34)

The historical aggregate Git tree at `39bfd1f0fcc33b677c137148b1e90ed847fe5a82` referenced the following exact submodule snapshots before the retirement. These references retain lineage without copying potentially insecure, obsolete, or differently licensed code into current Seamwork.

| Historic path | Canonical historical repository | Commit | Decision |
| --- | --- | --- | --- |
| `src/db` | [`ryjen/libcoda-db`](https://github.com/ryjen/libcoda-db) | `e1aa3f6f32aa7b3d875cd3182d19ff60dc359b71` | ARCHIVE — GPL-3.0; do not silently fold into MIT repository |
| `src/dice` | [`ryjen/libcoda-dice`](https://github.com/ryjen/libcoda-dice) | `4802a244a71186991057a3c71ce33a783c7b6277` | ABSORB selectively — C++23 randomness study is the active successor |
| `src/format` | [`ryjen/libcoda-format`](https://github.com/ryjen/libcoda-format) | `ed9e69323d8364b5f71bd6aae097637db608f2df` | ARCHIVE — parser/fuzz/test evidence |
| `src/net` | [`ryjen/libcoda-net`](https://github.com/ryjen/libcoda-net) | `5fd99d81f9c22bb9fa7f7de6a7ec8e2fb9ad5a43` | ARCHIVE — transport/TLS threat and test evidence |
| `cmake` | [`ryjen/cmake`](https://github.com/ryjen/cmake) | `84abe758c23959b8bff1a031b13c026376dcd4b5` | Retire unneeded build helpers |

The root `src/`, `tests/`, legacy `package.json`, and submodule metadata are historical only; none are current supported Seamwork products. Git history also preserves retired `json`, `log`, `math`, `string`, `terminal`, and `thread` experiments.

Reproduce legacy builds only by checking out an aggregate commit before #34 and initializing *its pinned* submodules. Do not interpret that ability as a supported or security-maintained release. The legacy aggregate and its generated/API reference material stay accessible through source history and independently archived repositories, not as default CI jobs. Component README redirects should remain available. GitHub archive settings and repository topic/description changes are administrative follow-ups, not reasons to retain old build code.

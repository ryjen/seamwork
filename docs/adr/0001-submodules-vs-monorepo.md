# ADR 0001 — Retain component repositories and submodules during modernization

## Status

**Superseded by [ADR 0002](0002-seamwork-topology.md).**

This decision remains as historical context for the `libcoda` modernization phase.

## Context

`libcoda` originally integrated independently versioned component repositories through Git submodules. The arrangement added operational complexity, but it represented the old project model: format, database, networking, dice, and other utilities were treated as independently reusable libraries.

During the earlier modernization phase, retaining those boundaries reduced risk while CMake contracts, tests, security boundaries, fuzzing, and packaging behavior were being characterized.

A monorepo migration at that point would have mixed source-control restructuring with behavioral modernization.

## Decision at the time

The project retained component repositories and Git submodules while modernization stabilized component behavior.

That sequencing decision enabled:

- component-local modernization and CI;
- explicit aggregate integration gates;
- gradual movement toward target-local CMake ownership;
- independent characterization of DB, network, format, and dice behavior;
- security and fuzzing work without requiring repository restructuring first.

## Why it is superseded

The project has now been re-chartered as **Seamwork**, a C++23 interface and architecture refinement laboratory rather than a collection of general-purpose utility libraries.

That changes the topology question.

Studies, examples, benchmarks, and promoted components benefit from atomic changes, while historical utility categories are no longer assumed to deserve independent release boundaries.

ADR 0002 therefore adopts a single primary Seamwork repository as the target topology, with incremental migration rather than immediate flattening.

## Historical alternatives

The original decision considered:

1. retaining repositories and submodules;
2. immediate monorepo migration;
3. package-only composition.

At the time, option 1 provided the clearest fault isolation with the least migration churn.

## Historical trade-off

The project accepted multi-repository coordination cost in exchange for lower modernization risk.

That trade-off was reasonable for the old phase and produced useful evidence. It is not binding on the re-chartered project.

## Consequence

Existing submodules remained during the Seamwork inventory/migration so useful code and provenance were not lost. They were subsequently retired from the active Seamwork tree under #34 once snapshots and dispositions had been recorded.

Their continued presence should be interpreted as transitional state governed by ADR 0002, not as endorsement of the old component topology.

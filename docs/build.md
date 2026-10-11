# Building Seamwork

## Active boundary

Seamwork is a self-contained C++23 interface/architecture study repository. Its default configuration does not need recursive submodules, external database servers, legacy utility libraries, or build-time fetched dependencies.

The historical libcoda aggregate has been retired from this repository's current tree. Its previous source, CMake integration and Git submodule pointers remain recoverable from Git history; the independently maintained historical component repositories remain available. For pinned revisions and license/provenance notes, see [the migration record](migration/seamwork-rebrand.md).

## Prerequisites

Use the checked-in flake on x86_64 Linux:

```bash
nix develop
```

This provides CMake, Ninja, GCC 14 and Clang. Without Nix, use CMake 3.21+ and a compiler with the C++23 facilities used by the studies.

## Development and release

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev

cmake --preset release
cmake --build --preset release
ctest --preset release
```

`SEAMWORK_BUILD_STUDIES`, `SEAMWORK_BUILD_EXAMPLES`, and `SEAMWORK_BUILD_TESTS` default to `ON`. Examples are maintained consumers of study interfaces, so `SEAMWORK_BUILD_EXAMPLES=ON` requires studies to be enabled. Active targets declare `cxx_std_23` and own their include and link requirements.

## CI and qualification

Required CI builds and runs active study tests with GCC and Clang in the flake environment. The study check target is available independently of CTest registration:

```bash
cmake --build --preset dev --target seamwork_study_checks
```

Legacy aggregate CI and `SEAMWORK_BUILD_LEGACY` are intentionally retired. To investigate historical behavior, check out the prior aggregate commit and its exact submodule SHAs from the [migration record](migration/seamwork-rebrand.md), then use its recorded compatibility environment. That historical graph has older and potentially insecure dependencies and must not be presented as an actively maintained library.

## Dependency/security posture

New studies should avoid silent network fetches, make ownership and error semantics explicit, and establish deterministic checks before promotion. See the [dependency policy](architecture/0004-dependency-policy.md), [charter](charter.md), and [refinement workflow](refinement-workflow.md).

# Building Seamwork

## Active boundary

Seamwork is a self-contained C++23 interface/architecture study repository with selectively promoted components. Its default configuration does not need recursive submodules, external database servers, legacy utility libraries, or build-time fetched dependencies.

The historical libcoda aggregate has been retired from this repository's current tree. Its previous source, CMake integration and Git submodule pointers remain recoverable from Git history; the independently maintained historical component repositories remain available. For pinned revisions and license/provenance notes, see [the migration record](migration/seamwork-rebrand.md).

## Prerequisites

Use the checked-in flake on x86_64 Linux:

```bash
nix develop
```

This provides CMake, Ninja, GCC 14, Clang and clang-tidy from the same pinned Nix development environment. Without Nix, use CMake 3.21+ and a compiler with the C++23 facilities used by the studies.

## Development and release

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev

cmake --preset release
cmake --build --preset release
ctest --preset release
```

Promoted components are part of the active build unconditionally. `SEAMWORK_BUILD_STUDIES`, `SEAMWORK_BUILD_EXAMPLES`, and `SEAMWORK_BUILD_TESTS` default to `ON`. Examples are maintained consumers of study interfaces, so `SEAMWORK_BUILD_EXAMPLES=ON` requires studies to be enabled. Active targets declare `cxx_std_23` and own their include and link requirements.

The first promoted component is the Yahtsee domain value contract under `components/yahtsee-domain`. Component tests are registered with CTest when `SEAMWORK_BUILD_TESTS=ON`.

## CI and qualification

Required CI builds and runs active study tests with GCC and Clang in the flake environment. The study check target is available independently of CTest registration:

```bash
cmake --build --preset dev --target seamwork_study_checks
```

Legacy aggregate CI and `SEAMWORK_BUILD_LEGACY` are intentionally retired. To investigate historical behavior, check out the prior aggregate commit and its exact submodule SHAs from the [migration record](migration/seamwork-rebrand.md), then use its recorded compatibility environment. That historical graph has older and potentially insecure dependencies and must not be presented as an actively maintained library.

## C++23 static analysis

The Clang CI job runs the checked-in `.clang-tidy` rules against the active
components, studies and maintained examples, using the CMake compilation
database. Configure with the Clang toolchain, then run the exact CI command:

```bash
nix develop --command env CC=clang CXX=clang++ cmake --preset release -G Ninja
nix develop --command ./scripts/check-static-analysis.sh build/release
```

The analysis script fails if the compilation database is missing, if clang-tidy
cannot analyze a translation unit, or if an enabled diagnostic is emitted.
The initial narrow gate checks Clang static-analyzer core and C++ analyses
plus use-after-move; it intentionally avoids auto-formatting and broad
style rewrites. Widen rules after reviewing baseline diagnostics, not by
silently suppressing failures. The active Clang CI check is the gate;
no new required-status context or runner is introduced.

The script processes active default-build `.cpp` files under
`components/`, `studies/` and `examples/`, including their project
headers. Optional fuzz translation units have their separate sanitizer
qualification and are not analyzed by this step.

## Dependency/security posture

New studies should avoid silent network fetches, make ownership and error semantics explicit, and establish deterministic checks before promotion. See the [dependency policy](architecture/0004-dependency-policy.md), [charter](charter.md), and [refinement workflow](refinement-workflow.md).


## Fuzz qualification

Pure trust-boundary parser fuzzers are opt-in and are not part of the normal GCC/Clang study matrix.

Configure them with Clang:

```bash
CC=clang CXX=clang++ cmake -S . -B build/fuzz -G Ninja \
  -DSEAMWORK_BUILD_FUZZERS=ON \
  -DSEAMWORK_BUILD_TESTS=OFF \
  -DSEAMWORK_BUILD_EXAMPLES=OFF

cmake --build build/fuzz --target \
  seamwork_fuzz_borrowed_frame \
  seamwork_fuzz_percent_decoding
```

The targets use libFuzzer with AddressSanitizer and UndefinedBehaviorSanitizer.

The separate `.github/workflows/fuzz.yml` workflow is intentionally path-scoped. It runs on changes to the fuzz surface, the two parser studies, or shared toolchain/CMake configuration and can also be triggered manually. The SPSC TSan workflow similarly includes shared toolchain and CMake inputs. Unrelated pull requests do not pay these sanitizer costs.

See [`fuzz/README.md`](../fuzz/README.md) for harness invariants and local usage.

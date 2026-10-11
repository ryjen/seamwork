# 0004 — Dependency policy

## Status

Historical libcoda modernization guidance. Active Seamwork studies use the independent C++23 build and the general no-implicit-fetch rule.

## Goals

Dependency management should make libcoda builds reproducible, reviewable, and usable offline after required dependencies are provisioned. Optional component features should not silently turn a normal compile into a network operation.

## Dependency order of preference

For normal library builds, prefer dependencies in this order:

1. **CMake imported/config targets supplied by the build environment** — for example `Threads::Threads`, `cereal::cereal`, OpenSSL targets, or package-provided targets where available.
2. **System/package-manager dependencies discovered by CMake** — appropriate for common native dependencies such as SQLite, PostgreSQL/libpq, MySQL/MariaDB client libraries, curl, json-c, and uriparser.
3. **Explicitly pinned fetched/vendored dependencies** — allowed only when the user intentionally enables that mode or when a hermetic packaging workflow owns the fetch outside the normal build.

A normal `cmake --preset dev` or `release` path should not require arbitrary network access to GitHub or another package source.

## Build-time network policy

### Default

Build-time dependency downloads are **off by default** for migrated code.

If a component retains an `ExternalProject`, `FetchContent`, package-manager bootstrap, or equivalent compatibility path, it must:

- be behind an explicit option such as `CODA_FETCH_DEPS=ON`;
- pin an immutable tag/commit/version;
- document what is downloaded and why;
- avoid changing the dependency graph when the option is off;
- remain non-required for aggregate CI unless the CI job exists specifically to test the fallback.

### Rationale

Implicit downloads create several risks:

- upstream compromise or tag drift enters the build trust boundary;
- offline/restricted builds fail unpredictably;
- nested configure/build operations inherit incompatible toolchain policy;
- dependency failures are misdiagnosed as libcoda source failures;
- CI caching can hide undeclared network requirements.

## CMake target policy

Prefer target-scoped dependency declarations:

- `target_link_libraries` with imported/owned targets;
- `target_include_directories` with `BUILD_INTERFACE`/`INSTALL_INTERFACE` for public headers;
- `target_compile_definitions` and `target_compile_options` rather than global flags;
- optional backend targets created only when their dependency is available/enabled;
- install/export lists composed from targets that actually exist.

Avoid using directory-global include/link state to make a component compile accidentally.

## Public versus private dependencies

A dependency is `PUBLIC` only when its headers/types or compile requirements are required to consume the public target. Otherwise it should be `PRIVATE`.

Examples:

- a header-only library included by a public header may need to be public until that header dependency is removed;
- PostgreSQL implementation headers used only by a `.cpp` adapter should remain private even if the adapter target itself is optional;
- test frameworks are test-only dependencies and must not become transitive requirements of the library target.

## Dependency versions

Pin versions when libcoda owns the download/vendor decision. For system/package-manager dependencies, document practical minimums only when code or API usage actually requires them.

Do not add a version pin solely for aesthetic consistency if the component already builds correctly against a supported distribution range.

## Git submodules

Project component submodules are not equivalent to third-party dependency downloads. They are explicit repository integration pins and are initialized by the checkout/integration workflow.

Rules:

- aggregate commits pin exact component SHAs through gitlinks;
- component changes should be validated in their own repository before or together with aggregate integration;
- advancing a gitlink requires aggregate CI evidence;
- third-party libraries should not be added as new Git submodules by default; prefer package/imported-target integration unless vendoring has a documented reason.

## Security expectations

For fetched or vendored third-party code:

- pin immutable revisions;
- minimize execution of upstream build/bootstrap scripts;
- review license and maintenance status;
- keep the dependency optional if the capability is optional;
- include dependency/version provenance in release/SBOM work when introduced.

For system packages:

- CI should pin the runner/distribution when package names or ABI expectations are distribution-specific;
- avoid silently depending on headers from development/server packages that are not declared in build documentation or CI provisioning.

## Test dependencies

Unit-test dependencies should not be configured or fetched when tests are disabled. Migrated components should honor the shared `CODA_BUILD_TESTS` cache option when built under the aggregate repository.

Service dependencies such as MySQL/PostgreSQL servers belong to explicit integration-test jobs or fixtures. Pure unit/fuzz paths must not require those services.

## Review checklist

When adding/changing a dependency:

- Is it required by the public target, a private adapter, tests, or tooling?
- Can it be represented as an imported CMake target?
- Does a normal build gain a new network operation?
- Is the version/provenance decision explicit?
- Can the dependency be disabled with the feature that requires it?
- Does CI provision it deterministically?
- Does it cross a security/licensing boundary that needs documentation?

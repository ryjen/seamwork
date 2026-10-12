# Changelog

Seamwork uses project-specific release tags so the re-chartered version line remains distinct from historical libcoda/libarg3 tags.

## Seamwork 0.1.0

Initial release of the re-chartered Seamwork project.

### Project identity

- renamed/re-chartered from the historical `libcoda` utility aggregate;
- active code requires C++23;
- archived libcoda submodules and legacy aggregate build are no longer part of the active checkout;
- design studies, practical consumers, evidence, and selectively promoted components are the product.

### Studies and evidence

- Yahtsee randomness/dependency-inversion comparison;
- recoverable validation/error boundary with `std::expected`;
- typed Yahtsee turn-state representation;
- direct C++23 standard formatting instead of a custom formatter abstraction;
- borrowed byte parsing and bounded lifetime semantics;
- explicit transaction commit with fail-safe rollback;
- authority identity separated from resolved transport endpoints;
- checked native size narrowing;
- bounded percent decoding with typed malformed-input errors;
- structured delayed work with `std::jthread` ownership/cancellation;
- bounded SPSC queue study driven by realtime DSP consumers.

### Promoted component

- `seamwork::yahtsee` domain values:
  - six-value `face`;
  - fixed five-face `hand`;
  - raw-input validation with typed errors.

This is intentionally narrow. Randomness, scoring, turn state, and application/scorecard behavior remain outside the component.

### Practical consumers

- composed Yahtsee turn;
- two-turn Yahtsee scorecard consumer.

These consumers supplied the evidence for the first component promotion.

### Qualification

- required GCC + Clang C++23 CI;
- deterministic study tests;
- path-scoped libFuzzer + ASan + UBSan qualification for pure parser boundaries;
- path-scoped ThreadSanitizer qualification for the SPSC concurrency study.

### Release scope

Seamwork 0.1.0 does **not** promise a broad stable utility-library API.

Studies remain experimental evidence. Promoted components are expected to meet a higher stability bar, but the project is still pre-1.0.

## Historical releases

The repository retains a historical tag/release:

- `0.5.1` / `libarg3-0.5.1` (2015).

It predates Seamwork and belongs to the repository's libcoda/libarg3 lineage. It is not part of the Seamwork semantic version sequence and is intentionally preserved for provenance.

The first Seamwork release tag is therefore namespaced as:

```text
seamwork-0.1.0
```

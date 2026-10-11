# Parser fuzz qualification

These harnesses provide adversarial evidence for Seamwork's two pure untrusted-input parser studies:

- borrowed frame parsing;
- percent decoding.

They are deliberately outside the normal study checks and CTest path.

## Build

Fuzzing is opt-in and Clang-only:

```bash
CC=clang CXX=clang++ cmake -S . -B build/fuzz -G Ninja \
  -DSEAMWORK_BUILD_FUZZERS=ON \
  -DSEAMWORK_BUILD_TESTS=OFF \
  -DSEAMWORK_BUILD_EXAMPLES=OFF

cmake --build build/fuzz --target \
  seamwork_fuzz_borrowed_frame \
  seamwork_fuzz_percent_decoding
```

The harness targets link libFuzzer, AddressSanitizer, and UndefinedBehaviorSanitizer through compiler flags.

## Harness invariants

### Borrowed frame

On successful parsing the harness asserts:

- consumed bytes stay within the supplied input;
- payload length stays within the configured limit;
- consumed bytes equal header plus payload length;
- the borrowed payload starts inside the original input at the documented header offset;
- explicit retention copies exactly the borrowed payload and consumed count.

It exercises several configured payload limits for every fuzzer input.

### Percent decoding

Both plus policies are exercised.

On success, decoded output cannot be longer than encoded input.

On failure, the typed error index must refer to a byte inside the supplied input.

## CI policy

The fuzz workflow is path-scoped. It runs for changes to:

- these harnesses/corpora;
- borrowed-frame;
- percent-decoding;
- fuzz build configuration.

It can also be triggered manually.

It does not run for unrelated Seamwork pull requests.

The CI job is a bounded smoke qualification, not an open-ended fuzzing service. Longer local fuzz sessions remain opt-in.

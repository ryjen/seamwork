#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

build_dir="${1:-build/release}"
if [[ ! -s "$build_dir/compile_commands.json" ]]; then
    printf 'Missing CMake compilation database: %s/compile_commands.json\n' "$build_dir" >&2
    exit 1
fi

if ! command -v clang-tidy >/dev/null 2>&1; then
    printf 'clang-tidy is unavailable; enter the pinned Nix development shell\n' >&2
    exit 1
fi

# Only default-build translation units. Opt-in fuzz targets have a separate
# sanitizer qualification and do not belong to this Clang analysis gate.
mapfile -d '' sources < <(
    find components studies examples -type f -name '*.cpp' -print0 | sort -z
)

if (( ${#sources[@]} == 0 )); then
    printf 'No Seamwork C++23 source files found\n' >&2
    exit 1
fi

clang-tidy --config-file="$repo_root/.clang-tidy" \
    -p="$build_dir" "${sources[@]}"

# QART: explicit commit and fail-safe transaction lifetime

## Question

How should a transaction-like resource express ownership, success, cleanup, and failure?

## Historical baseline

Archived `libcoda-db::transaction` was copyable and held shared pointers to session/backend state. Each copy also held its own `successful_` flag.

On destruction:

```text
if active:
  if successful -> commit()
  else          -> rollback()
```

Both commit and rollback could throw.

### Consequences

- multiple objects can believe they own the same active transaction;
- per-copy success intent can conflict;
- destruction order can determine commit versus rollback;
- commit is delayed/implicit rather than the operation whose failure the caller handles;
- throwing cleanup during unwinding has no safe ordinary error channel.

The test study includes a small copyable baseline showing one copy's destructor can roll back despite another copy intending commit.

## Alternatives

### A. Copyable + success flag

Historical shape.

Rejected because active transaction ownership is singular in the modeled domain. Copyability represents an ownership state the lifecycle does not support.

### B. Move-only guard with explicit commit

Selected.

The active transaction has one owning guard. Copy construction/assignment and move assignment are deleted; move construction transfers cleanup responsibility.

`commit()` returns a typed result. Successful commit marks the guard complete. Failed commit remains active so destruction attempts rollback.

**Benefits**

- single lifecycle owner;
- commit failure is visible where commit is requested;
- fail-safe default is rollback;
- exception/early-return paths get rollback automatically;
- repeat commit/rollback is rejected explicitly.

**Cost**

- a named type carries lifecycle state;
- destructor-time rollback failure still cannot be returned.

### C. `std::scope_exit`

A local rollback guard plus explicit commit is compact and, where available, uses the C++23 `std::scope_exit` facility.

This is attractive when transaction handling is entirely local and no transaction object needs to move through helper scopes.

The required Seamwork Clang environment currently lacks the `<scope>` header, so this alternative is conditionally compiled. The study treats that as portability evidence rather than adding a project-local scope-exit substitute.

A named move-only guard is more communicative when transaction ownership itself is an API concept and is portable across Seamwork's required toolchains.

### D. Commit-on-success destructor

Rejected.

Making destructor commit non-throwing would hide commit failure or require an out-of-band channel for the application's success operation. Commit is too important to defer to an error-suppressed cleanup path.

## Destructor failure policy

Rollback on abandonment is a safety cleanup action, but rollback itself can fail.

There is no general mechanism for a destructor to return that failure. Therefore:

- destructor remains `noexcept`;
- cleanup failure must go to a non-throwing observability channel;
- explicit `rollback()`, when the caller needs to know its result, returns a typed error normally.

The fake backend records cleanup failures solely to make this limitation testable.

## Why no real database?

The architectural question is lifecycle semantics, not SQL/backend behavior.

A deterministic fake backend gives stronger evidence for:

- destruction order;
- move ownership;
- injected commit/rollback failure;
- exception unwinding;

without introducing network/service uncertainty or reviving the archived DB abstraction.

## Recommendation

Use explicit commit and rollback-by-default ownership.

For local one-function work, `std::scope_exit` is a strong standard-library option. When an active transaction is itself a value that must move between scopes, use a move-only guard with explicit state transitions.

Do not promote a database or generic transaction component from this study alone.

# Structured delayed-work study

Issue #67 revisits the archived `coda::later` helper as negative lifetime evidence rather than as a component to port.

The historical helper could either block its constructor or start a detached thread that slept and invoked a bound task. The constructed `later` value did not own the detached work after construction, offered no cancellation or join handle, and represented milliseconds as a plain integer.

## Selected interface

The study models one delayed operation as one move-only owner:

```cpp
auto task = delayed_task::start(
    250ms,
    [](std::stop_token stop) noexcept {
        ...
    }
);
```

The explicit `start` operation makes activation visible.

The owner:

- stores a `std::jthread`;
- cannot be copied;
- can transfer ownership by move construction;
- accepts a `std::chrono::milliseconds` delay;
- can request cancellation;
- joins explicitly through `join()`;
- automatically requests stop and joins on destruction through `std::jthread`.

No thread is detached.

This does not make arbitrary captured references safe. If a callback captures external state by reference, that state must still outlive the `delayed_task`. The improvement is that the worker has an owner whose destruction joins it, so normal scope ordering can enforce that relationship.

## Interruptible delay

A plain `sleep_for` would make destruction wait for the original deadline even after cancellation.

The worker instead uses the stop-token-aware `std::condition_variable_any::wait_for` overload. This keeps stop registration and wakeup coupled to the wait operation, avoiding a lost notification if a stop request races with entering the wait.

That lets a one-hour pending delay be cancelled immediately in tests without sleeping for wall-clock time.

## Callback cancellation

If the delay expires and the callback begins, the owner cannot forcibly interrupt arbitrary code.

The callback therefore receives the same `std::stop_token` and may cooperate with later stop requests.

## Callback failure contract

This study intentionally accepts only callbacks invocable as:

```cpp
void(std::stop_token) noexcept
```

That keeps background exception semantics explicit. A result-bearing or throwing task would require a separate future/result channel rather than silent swallowing or thread-level termination.

## Alternatives

The QART compares:

- detached `std::thread`;
- `std::async`;
- owned `std::jthread`;
- an external scheduler/event loop.

## Promotion status

**Study only.**

A single delayed operation does not justify a scheduler, executor, or general thread component.

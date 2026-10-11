# QART: structured delayed work

## Question

How should one delayed operation express ownership, cancellation, duration, and destruction in modern C++?

## Historical baseline

The archived `coda::later` constructor accepted:

```cpp
later(int after, bool async, callable&& f, arguments&&... args)
```

For async work it created a thread, slept for the integer millisecond delay, invoked the bound task, and detached the thread.

For synchronous work it blocked the constructor, slept, and invoked the task inline.

### Consequences

- a boolean selects fundamentally different execution/lifetime models;
- asynchronous work has no owning value after construction;
- no join/cancel operation exists;
- captured references may outlive their source scope;
- destructor semantics say nothing about outstanding detached work;
- delay units live in documentation/convention rather than the type;
- a task exception on a detached worker has no caller-facing result channel.

## Alternatives

### A. Detached `std::thread`

This preserves fire-and-forget syntax.

Rejected as the default because work lifetime becomes independent from the initiating scope. Detachment can be correct for process-lifetime services, but this one delayed callback has no such requirement.

### B. `std::async`

A future provides result and exception propagation.

However, `std::async` does not directly model a cancellable delay, and its launch/future lifetime semantics answer a different question. It is attractive when the operation is primarily result-bearing rather than timer-like.

### C. Move-only `std::jthread` owner

Selected.

One object owns one worker. `std::jthread` requests stop and joins during destruction.

The study adds an interruptible wait before invoking the task and passes the stop token into the task for cooperative cancellation after execution begins.

### D. Scheduler/event loop

A scheduler becomes useful when many timers share threads, priorities, deadlines, queues, or event-loop integration.

One delayed operation is not enough evidence for that infrastructure.

## Activation

The study uses:

```cpp
delayed_task::start(...)
```

instead of doing visible work in an ordinary public constructor.

The returned value is already active, but the call site names the side effect explicitly.

## Delay ownership and cancellation

The worker owns the callback.

The `delayed_task` owns the worker.

Pending delay uses a condition-variable wait with a stop callback so stop requests wake the delay promptly.

On destruction, `std::jthread` requests stop and joins. Therefore no worker survives destruction of its owner.

## In-task cancellation

Once the callback begins, cancellation is cooperative.

Passing `std::stop_token` makes that fact explicit instead of pretending arbitrary user code can be forcibly interrupted safely.

## Failure policy

The callback is constrained to `noexcept`.

This is intentionally restrictive. It prevents the study from quietly choosing among:

- terminate on uncaught background exception;
- swallow/log;
- store `exception_ptr`;
- expose a future/result channel.

A future study with a real result-bearing consumer can compare those policies.

## Recommendation

For one delayed fire-and-forget operation whose lifetime belongs to a scope, prefer a move-only `std::jthread` owner with explicit cancellation and strongly typed duration.

Do not promote a scheduler from this evidence.

The core refinement is:

> delayed work should have an owner; destruction should end that ownership relationship rather than leave a detached thread behind.

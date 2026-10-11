# QART: authority identity versus transport endpoint

## Question

How should a secure-connection interface prevent DNS resolution output from being confused with the identity the caller intended to authenticate?

## Historical baseline

Archived `libcoda-net::socket::connect(host, port)`:

1. receives the caller hostname;
2. calls `getaddrinfo`;
3. stores the selected socket address;
4. attaches the TLS layer to the socket.

The secure-layer interface receives the socket descriptor but not the original hostname.

That means the architecture does not carry the authority identity explicitly across the trust boundary.

## Alternatives

### A. One mutable host string

A connection routine can overwrite/reuse `host` after resolution.

This is compact but destroys provenance: downstream code can no longer tell whether a string is caller authority or resolver output.

Rejected.

### B. Two named plain strings

```cpp
std::string requested_host;
std::string resolved_address;
```

This preserves both values and is already much better than overwriting one.

The remaining weakness is substitution: APIs taking `std::string_view` cannot tell whether the caller supplied the authority or transport address.

### C. Distinct semantic value types

Selected.

```cpp
authority_name
transport_address
```

The types are intentionally not convertible to one another.

A dialer receives only `resolved_endpoint`; a verifier receives only `authority_name`.

This turns an important security convention into a compile-time interface constraint.

### D. Verify the resolved address

This is correct only when the original caller authority itself is that IP literal.

Resolution output does not become an authentication identity merely because it is where the connection was routed.

## Why own the strings?

A borrowed view would reduce copies, but the authority is security-relevant context that often survives multiple phases:

```text
request -> resolve -> choose candidate -> connect -> verify
```

The study therefore chooses owning values. The strings are small compared with the cost/complexity of a connection, and no performance claim depends on avoiding that copy.

## Port handling

The authority and resolved endpoint both carry a port because they represent different facts:

- caller-requested service authority;
- concrete transport endpoint.

The study does not silently reconcile a mismatch. A real resolver/connection layer should define how service/redirect policy affects ports instead of hiding that rule inside these value types.

TLS identity consumes the authority name, not the transport address.

## Normalization

No hostname normalization, IDNA handling, certificate matching, or SNI behavior is implemented.

Those are security policies with standards-specific semantics. This study only preserves the original authority so such policy can be applied later at the correct boundary.

## Recommendation

Use separate value types for trust identity and routing identity when the consequences of confusing them are security-sensitive.

The design is deliberately smaller than a networking abstraction:

- no resolver interface;
- no socket;
- no TLS wrapper;
- no certificate verifier implementation;
- no URI parser.

The lesson is architectural:

> resolution produces an endpoint; it must not erase or replace the authority being authenticated.

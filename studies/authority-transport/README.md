# Authority vs transport identity study

Issue #61 extracts a security/interface seam from archived `libcoda-net` without adding networking dependencies.

The historical client connection path accepted a hostname, resolved it to an address, connected to that address, and then attached TLS. The retained threat model identifies the missing contract: the caller hostname must remain available for peer authentication because resolution does not establish identity.

## Selected design

Keep two semantic values distinct:

```cpp
authority_name
transport_address
```

and compose them into a value-oriented plan:

```cpp
secure_connection_plan {
    requested_authority authority;
    resolved_endpoint transport;
}
```

The fake transport boundary accepts only a `resolved_endpoint`.

The fake verification boundary accepts only an `authority_name`.

That makes the intended dependency visible:

```text
caller authority -----------------------> peer verification
       |
       +--> resolution --> transport ---> dial/connect
```

Resolution adds a routing result; it does not replace caller identity.

## Ownership

The study uses owning `std::string` values rather than borrowed views.

The authority may need to survive resolution, candidate selection, connection setup, and verification. Owning this small piece of security-relevant context avoids introducing an unrelated lifetime dependency between those phases.

No hostname canonicalization is performed. The study preserves what the caller requested rather than inventing certificate-verification policy.

## Multiple candidates

A single `requested_authority` can be copied into plans for multiple IPv4/IPv6 candidates. Each transport endpoint may differ while the verification identity remains unchanged.

## IP literals

If the caller originally requests an IP literal, that literal remains the authority identity.

The design never infers "verify the resolved address" merely because a hostname happened to resolve to it.

## Build

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev -R seamwork_authority_transport
./build/dev/studies/authority-transport/seamwork_authority_transport_example
```

## Promotion status

**Study only.**

No DNS, TLS, URI, socket, or networking framework is introduced.

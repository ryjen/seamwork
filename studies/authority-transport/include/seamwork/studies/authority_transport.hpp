#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace seamwork::studies::authority_transport {

class authority_name
{
public:
    explicit authority_name(std::string value)
        : value_(std::move(value))
    {
    }

    [[nodiscard]] auto value() const noexcept -> std::string_view
    {
        return value_;
    }

    friend auto operator==(const authority_name&, const authority_name&) -> bool = default;

private:
    std::string value_;
};

class transport_address
{
public:
    explicit transport_address(std::string value)
        : value_(std::move(value))
    {
    }

    [[nodiscard]] auto value() const noexcept -> std::string_view
    {
        return value_;
    }

    friend auto operator==(const transport_address&, const transport_address&) -> bool = default;

private:
    std::string value_;
};

struct requested_authority {
    authority_name name;
    std::uint16_t port{};
};

struct resolved_endpoint {
    transport_address address;
    std::uint16_t port{};
};

struct secure_connection_plan {
    requested_authority authority;
    resolved_endpoint transport;
};

[[nodiscard]] inline auto plan_connection(
    const requested_authority& authority,
    const resolved_endpoint& transport
) -> secure_connection_plan
{
    return secure_connection_plan{
        .authority = authority,
        .transport = transport,
    };
}

class fake_dialer
{
public:
    void connect(const resolved_endpoint& endpoint)
    {
        last_address_ = std::string{endpoint.address.value()};
        last_port_ = endpoint.port;
        connected_ = true;
    }

    [[nodiscard]] auto connected() const noexcept -> bool
    {
        return connected_;
    }

    [[nodiscard]] auto last_address() const noexcept -> std::string_view
    {
        return last_address_;
    }

    [[nodiscard]] auto last_port() const noexcept -> std::uint16_t
    {
        return last_port_;
    }

private:
    std::string last_address_;
    std::uint16_t last_port_{};
    bool connected_{};
};

class fake_peer_verifier
{
public:
    void verify(const authority_name& identity)
    {
        last_identity_ = std::string{identity.value()};
        verified_ = true;
    }

    [[nodiscard]] auto verified() const noexcept -> bool
    {
        return verified_;
    }

    [[nodiscard]] auto last_identity() const noexcept -> std::string_view
    {
        return last_identity_;
    }

private:
    std::string last_identity_;
    bool verified_{};
};

inline void execute_secure_plan(
    const secure_connection_plan& plan,
    fake_dialer& dialer,
    fake_peer_verifier& verifier
)
{
    dialer.connect(plan.transport);
    verifier.verify(plan.authority.name);
}

namespace alternatives {

struct plain_string_plan {
    std::string requested_host;
    std::string resolved_address;
    std::uint16_t port{};
};

} // namespace alternatives

} // namespace seamwork::studies::authority_transport

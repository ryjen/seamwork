#include <iostream>

#include <seamwork/studies/authority_transport.hpp>

namespace sw = seamwork::studies::authority_transport;

int main()
{
    const sw::requested_authority requested{
        .name = sw::authority_name{"api.example.test"},
        .port = 443,
    };

    const auto plan = sw::plan_connection(
        requested,
        sw::resolved_endpoint{
            .address = sw::transport_address{"203.0.113.17"},
            .port = 443,
        }
    );

    sw::fake_dialer dialer;
    sw::fake_peer_verifier verifier;
    sw::execute_secure_plan(plan, dialer, verifier);

    std::cout << "dial: " << dialer.last_address() << ':' << dialer.last_port() << '\n'
              << "verify as: " << verifier.last_identity() << '\n';
}

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include <seamwork/studies/authority_transport.hpp>

namespace sw = seamwork::studies::authority_transport;

namespace {

void require(bool condition, std::string_view message)
{
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

static_assert(!std::is_convertible_v<sw::authority_name, sw::transport_address>);
static_assert(!std::is_convertible_v<sw::transport_address, sw::authority_name>);
static_assert(!std::is_constructible_v<sw::authority_name, sw::transport_address>);
static_assert(!std::is_constructible_v<sw::transport_address, sw::authority_name>);

void test_hostname_authority_is_preserved_across_resolution()
{
    const sw::requested_authority requested{
        .name = sw::authority_name{"api.example.test"},
        .port = 443,
    };
    const sw::resolved_endpoint resolved{
        .address = sw::transport_address{"203.0.113.17"},
        .port = 443,
    };

    const auto plan = sw::plan_connection(requested, resolved);

    sw::fake_dialer dialer;
    sw::fake_peer_verifier verifier;
    sw::execute_secure_plan(plan, dialer, verifier);

    require(dialer.connected(), "transport should be dialed");
    require(dialer.last_address() == "203.0.113.17", "dialer should receive resolved address");
    require(dialer.last_port() == 443, "dialer should receive transport port");
    require(verifier.verified(), "peer identity should be verified");
    require(
        verifier.last_identity() == "api.example.test",
        "verifier should receive original caller authority"
    );
}

void test_multiple_candidates_share_one_authority()
{
    const sw::requested_authority requested{
        .name = sw::authority_name{"service.example.test"},
        .port = 443,
    };

    const auto first = sw::plan_connection(
        requested,
        sw::resolved_endpoint{
            .address = sw::transport_address{"2001:db8::10"},
            .port = 443,
        }
    );
    const auto second = sw::plan_connection(
        requested,
        sw::resolved_endpoint{
            .address = sw::transport_address{"198.51.100.24"},
            .port = 443,
        }
    );

    require(
        first.authority.name == second.authority.name,
        "resolution candidates should preserve the same authority identity"
    );
    require(
        first.transport.address != second.transport.address,
        "resolution candidates may differ in transport address"
    );
}

void test_ip_literal_authority_remains_identity()
{
    const sw::requested_authority requested{
        .name = sw::authority_name{"192.0.2.44"},
        .port = 443,
    };
    const sw::resolved_endpoint resolved{
        .address = sw::transport_address{"192.0.2.44"},
        .port = 443,
    };

    const auto plan = sw::plan_connection(requested, resolved);

    sw::fake_dialer dialer;
    sw::fake_peer_verifier verifier;
    sw::execute_secure_plan(plan, dialer, verifier);

    require(
        verifier.last_identity() == "192.0.2.44",
        "IP-literal authority should remain the verification identity"
    );
}

void test_plain_strings_do_not_prevent_field_confusion()
{
    const sw::alternatives::plain_string_plan baseline{
        .requested_host = "api.example.test",
        .resolved_address = "203.0.113.17",
        .port = 443,
    };

    const auto accidentally_selected_for_verification = baseline.resolved_address;

    require(
        accidentally_selected_for_verification != baseline.requested_host,
        "plain strings rely on caller discipline to choose the correct identity"
    );
}

} // namespace

int main()
{
    try {
        test_hostname_authority_is_preserved_across_resolution();
        test_multiple_candidates_share_one_authority();
        test_ip_literal_authority_remains_identity();
        test_plain_strings_do_not_prevent_field_confusion();
    } catch (const std::exception& error) {
        std::cerr << "authority-transport study failed: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

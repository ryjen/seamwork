#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <span>

#include <seamwork/studies/borrowed_frame.hpp>

namespace sw = seamwork::studies::borrowed_frame;

namespace {

void verify_parse(
    std::span<const std::byte> input,
    std::size_t max_payload
)
{
    const auto parsed = sw::parse(input, max_payload);
    if (!parsed) {
        return;
    }

    if (parsed->consumed > input.size()) {
        std::abort();
    }
    if (parsed->payload.size() > max_payload) {
        std::abort();
    }
    if (parsed->consumed != sw::header_size + parsed->payload.size()) {
        std::abort();
    }
    if (parsed->payload.data() != input.data() + sw::header_size) {
        std::abort();
    }

    const auto owned = sw::retain(*parsed);
    if (owned.consumed != parsed->consumed) {
        std::abort();
    }
    if (!std::ranges::equal(owned.payload, parsed->payload)) {
        std::abort();
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    const auto input = std::span<const std::byte>{
        reinterpret_cast<const std::byte*>(data),
        size,
    };

    for (const auto limit : std::array<std::size_t, 4>{
             0,
             1,
             32,
             sw::default_max_payload,
         }) {
        verify_parse(input, limit);
    }

    return 0;
}

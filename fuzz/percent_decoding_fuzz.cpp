#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string_view>

#include <seamwork/studies/percent_decoding.hpp>

namespace sw = seamwork::studies::percent_decoding;

namespace {

void verify_decode(std::string_view input, sw::plus_policy plus)
{
    const auto decoded = sw::decode_percent(input, plus);

    if (decoded) {
        if (decoded->size() > input.size()) {
            std::abort();
        }
        return;
    }

    if (input.empty() || decoded.error().index >= input.size()) {
        std::abort();
    }
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size)
{
    const auto input = std::string_view{
        reinterpret_cast<const char*>(data),
        size,
    };

    verify_decode(input, sw::plus_policy::literal);
    verify_decode(input, sw::plus_policy::space);

    return 0;
}

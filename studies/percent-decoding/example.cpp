#include <iostream>

#include <seamwork/studies/percent_decoding.hpp>

namespace sw = seamwork::studies::percent_decoding;

int main()
{
    const auto decoded = sw::decode_percent(
        "hello%20world+again",
        sw::plus_policy::space
    );

    if (!decoded) {
        std::cerr << "decode failed at input index " << decoded.error().index << '\n';
        return 2;
    }

    std::cout << *decoded << '\n';
}

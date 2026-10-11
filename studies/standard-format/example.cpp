#include <iostream>
#include <iterator>
#include <string>

#include <seamwork/studies/standard_format.hpp>

namespace sw = seamwork::studies::standard_format;

int main()
{
    const sw::diagnostic sample{
        .component = "worker",
        .latency_ms = 7.25,
        .code = 0xAF,
    };

    std::cout << sw::render_diagnostic(sample) << '\n';

    std::string owned = "buffered: ";
    sw::append_diagnostic(std::back_inserter(owned), sample);
    std::cout << owned << '\n';
}

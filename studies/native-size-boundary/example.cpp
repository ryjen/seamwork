#include <array>
#include <cstddef>
#include <iostream>

#include <seamwork/studies/native_size_boundary.hpp>

namespace sw = seamwork::studies::native_size_boundary;

int main()
{
    const std::array bytes{
        std::byte{0xDE},
        std::byte{0xAD},
        std::byte{0xBE},
        std::byte{0xEF},
    };

    sw::fake_native_sink sink;
    sink.return_next(2);

    const auto written = sw::write_once(bytes, sink);
    if (!written) {
        std::cerr << "native write rejected\n";
        return 2;
    }

    std::cout << "requested: " << bytes.size()
              << "\nnative count: " << sink.last_count()
              << "\nwritten: " << *written << '\n';
}

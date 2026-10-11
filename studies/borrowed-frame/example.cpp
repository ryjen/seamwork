#include <array>
#include <cstddef>
#include <iostream>
#include <string>

#include <seamwork/studies/borrowed_frame.hpp>

namespace sw = seamwork::studies::borrowed_frame;

constexpr auto b(unsigned value) -> std::byte
{
    return static_cast<std::byte>(value);
}

auto as_text(std::span<const std::byte> bytes) -> std::string
{
    std::string value;
    value.reserve(bytes.size());

    for (const auto byte : bytes) {
        value.push_back(static_cast<char>(std::to_integer<unsigned char>(byte)));
    }

    return value;
}

int main()
{
    std::array input{
        b(0x00), b(0x05),
        b('h'), b('e'), b('l'), b('l'), b('o'),
        b(0x00), b(0x01), b('!'),
    };

    const auto parsed = sw::parse(input, 64);
    if (!parsed) {
        std::cerr << "frame rejected\n";
        return 2;
    }

    std::cout << "borrowed now: " << as_text(parsed->payload) << '\n';

    const auto retained = sw::retain(*parsed);
    input[2] = b('H');

    std::cout << "caller changed buffer: " << as_text(parsed->payload) << '\n';
    std::cout << "explicit retained copy: " << as_text(retained.payload) << '\n';
}

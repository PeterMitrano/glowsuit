#include <iostream>
#include <common.h>

int main()
{
    auto const bytes = to_bytes<unsigned long>(21232642ul);
    for (auto const &byte : bytes)
    {
        std::cout << std::hex << static_cast<int>(byte) << '\n';
    }
    return EXIT_SUCCESS;
}
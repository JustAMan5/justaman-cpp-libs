#include <cstdint>
#include <justaman/memory/byte_buffer.hpp>
#include <justaman/io/logger.hpp>



int main()
{
    jman::byte_buffer<256> buffer {};

    buffer.write_string("hello yessir");

    jman::println("{}", buffer.readable_bytes());

    jman::println("real {}", buffer.read<float>());
    jman::println("real {}", buffer.read<std::uint32_t>());

    buffer.reset_indices();

    jman::println("inv {}", buffer.read_string(5));
    jman::println("inv {}", buffer.read<float>(5));
}
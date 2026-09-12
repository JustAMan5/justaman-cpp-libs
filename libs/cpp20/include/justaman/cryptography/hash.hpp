#pragma once

#include <cstddef>
namespace jman 
{
    template <typename T>
    struct fhash;

    template <>
    struct fhash<char>
    {
        std::size_t operator()(char value) {
            return static_cast<std::size_t>(value);
        }
    };
}
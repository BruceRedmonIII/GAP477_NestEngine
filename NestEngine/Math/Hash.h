#pragma once
#include <cstdint>

// HashedId is used to Identify a class and/or object by a unique number
using HashedId = uint64_t;
namespace nest
{

    constexpr inline HashedId fNVPrime = 0x00000100000001B3;
    constexpr inline HashedId fNVOffsetBasis = 0xcbf29ce484222325;

    /*algorithm fnv-1a is
    hash := FNV_offset_basis

    for each byte_of_data to be hashed do
        hash := hash XOR byte_of_data
        hash := hash × FNV_prime

    return hash */

    static inline constexpr HashedId Hash(const char* pString)
    {
        if (!pString)
            return 0;

        HashedId hash = fNVOffsetBasis;
        while (*pString != '\0')
        {
            hash ^= static_cast<HashedId>(*pString);
            hash *= fNVPrime;
            ++pString;
        }

        return hash;
    }
}
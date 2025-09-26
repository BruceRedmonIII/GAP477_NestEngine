#pragma once
#include <cstdint>
#include <string>

namespace nest
{
    struct TileSetData
    {
        std::string m_source{};
        uint16_t m_firstGrid = 0;
    };
}
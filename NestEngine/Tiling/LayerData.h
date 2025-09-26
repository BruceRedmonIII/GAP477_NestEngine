#pragma once
#include <cstdint>
#include <string>

namespace nest
{
    struct LayerData
    {
        uint16_t m_id = 0;
        std::string m_name{};
        uint16_t m_width = 0;
        uint16_t m_height = 0;
    };
}
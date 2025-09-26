#pragma once
#include "../lib/gap-311/VulkanWrapper.hpp"
namespace nest
{
    // A sampler is used to access the data from within the texture
    struct Sampler
    {
        vk::Sampler sampler;
    };
}
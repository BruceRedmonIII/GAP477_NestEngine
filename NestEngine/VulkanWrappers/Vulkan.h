#pragma once
#include "../lib/gap-311/VulkanWrapper.hpp"
#include "../lib/gap-311/VulkanShaderLibrary.hpp"
namespace nest
{
	struct Vulkan
	{
		GAP311::VulkanWrapper vulkan{};
		GAP311::VulkanShaderLibrary shaderLib{};
	};
}
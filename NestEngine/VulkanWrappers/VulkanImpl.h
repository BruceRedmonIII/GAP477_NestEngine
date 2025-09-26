#pragma once
#include "../lib/gap-311/VulkanWrapper.hpp"
#include "../lib/gap-311/VulkanShaderLibrary.hpp"
namespace nest
{
	struct VulkanImpl
	{
		GAP311::VulkanWrapper m_vulkan{};
		GAP311::VulkanShaderLibrary m_shaderLib{};
		~VulkanImpl()
		{
			m_shaderLib.Shutdown();
			m_vulkan.Shutdown();
		}
	};
}
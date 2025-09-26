#include "ResourceManager.h"
#include "../Engine/Engine.h"
#include "GraphicsSystem.h"
#include "../VulkanWrappers/Vulkan.h"
#include "../VulkanWrappers/Texture.h"
#include "../lib/BleachNew/BleachNew.h"
GAP311::ShaderUniform nest::ResourceManager::s_diffuseUniform{};
GAP311::ShaderUniform nest::ResourceManager::s_normalUniform{};
GAP311::ShaderUniform nest::ResourceManager::s_roughnessUniform{};
GAP311::ShaderUniform nest::ResourceManager::s_metallicUniform{};
GAP311::ShaderUniform nest::ResourceManager::s_ambientOcclusion{};
GAP311::ShaderUniform nest::ResourceManager::s_opacityUniform{};

bool nest::ResourceManager::Init()
{
	s_diffuseUniform = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformForImage(3, nullptr, nullptr);
	s_normalUniform = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformForImage(4, nullptr, nullptr);
	s_roughnessUniform = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformForImage(5, nullptr, nullptr);
	s_metallicUniform = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformForImage(6, nullptr, nullptr);
	s_ambientOcclusion = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformForImage(7, nullptr, nullptr);
	s_opacityUniform = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateUniformForImage(8, nullptr, nullptr);
	return true;
}

void nest::ResourceManager::Exit()
{
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().waitIdle();
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyUniform(s_diffuseUniform);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyUniform(s_normalUniform);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyUniform(s_roughnessUniform);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyUniform(s_metallicUniform);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyUniform(s_ambientOcclusion);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyUniform(s_opacityUniform);
	for (auto& texture : m_textures)
	{
		BLEACH_DELETE(texture.second);
		texture.second = nullptr;
	}
}
int nest::ResourceManager::AddTexture(const std::string& path, aiMaterial* material)
{
	PBRTextureStack* texture = BLEACH_NEW(PBRTextureStack(path, material));
	if (texture->IsValid())
	{
		++s_textureCounter;
		m_textures.try_emplace(s_textureCounter, texture);
		return s_textureCounter;
	}
	else
	{
		BLEACH_DELETE(texture);
	}
	return s_kInvalidIndex;
}

void nest::ResourceManager::AddMaterial(int index, Material mat)
{
	m_materialMap.try_emplace(index, mat);
}

nest::PBRTextureStack* nest::ResourceManager::GetTexture(int index)
{
	// we should avoid ever calling GetTexture with an invalid texture 
	// as it would be called every frame
	//assert(index != s_kInvalidIndex);
	return m_textures[index];
}
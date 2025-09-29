#include "PipelineConfigs.h"
#include "../GlobalConstants/constants.h"
#include "../Managers/GraphicsSystem.h"
#include "../Managers/LightManager.h"
#include "../Managers/SkyboxManager.h"
#include "../Uniforms/MeshDataStructs.h"
#include "../VulkanWrappers/Vulkan.h"
#include "Pipeline.h"
#include "../Engine/Engine.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../Managers/CameraManager.h"
#include "../Managers/ResourceManager.h"

nest::Pipeline* nest::CreateMeshPipeline()
{
	return BLEACH_NEW(Pipeline(s_kMeshPipeline, nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreatePipeline(
		{
				.vertexBindings =
				{
					vk::VertexInputBindingDescription()
						.setBinding(0)
						.setInputRate(vk::VertexInputRate::eVertex)
						.setStride(sizeof(nest::MeshVertex))
				},
				.vertexAttributes =
				{
					vk::VertexInputAttributeDescription()
						.setLocation(0)
						.setBinding(0)
						.setFormat(vk::Format::eR32G32B32Sfloat)
						.setOffset(offsetof(MeshVertex, MeshVertex::position)),
					vk::VertexInputAttributeDescription()
						.setLocation(1)
						.setBinding(0)
						.setFormat(vk::Format::eR32G32B32Sfloat)
						.setOffset(offsetof(MeshVertex, MeshVertex::normal)),
					vk::VertexInputAttributeDescription()
						.setLocation(2)
						.setBinding(0)
						.setFormat(vk::Format::eR32G32Sfloat)
						.setOffset(offsetof(MeshVertex, MeshVertex::tangent)),
					vk::VertexInputAttributeDescription()
						.setLocation(3)
						.setBinding(0)
						.setFormat(vk::Format::eR32G32Sfloat)
						.setOffset(offsetof(MeshVertex, MeshVertex::bitangent)),
				},
				.shaderPushConstants =
				{
					vk::PushConstantRange()
						.setOffset(0)
						.setSize(sizeof(PushConstants))
						.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
				},
				.shaderUniforms =
				{
					nest::CameraManager::s_cameraUniform,
					nest::LightManager::s_lightUniform,
					nest::LightManager::s_shadowMapUniform,
				},
				.shaderStages =
				{
					vk::PipelineShaderStageCreateInfo()
						.setStage(vk::ShaderStageFlagBits::eVertex)
						.setPName(s_kMainName)
						.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule("mesh.vert")),
					vk::PipelineShaderStageCreateInfo()
						.setStage(vk::ShaderStageFlagBits::eFragment)
						.setPName(s_kMainName)
						.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule("mesh.frag"))
				},
				.polygonMode = s_polygonMode,
				.cullMode = s_cullMode,
				.frontFace = s_frontFace,
				.depthTestEnable = s_depthTest,
				.depthCompareOp = s_compareOp,
				.depthWriteEnable = s_depthWrite,
				.colorBlendStates =
				{
				vk::PipelineColorBlendAttachmentState()
					.setBlendEnable(true)
					.setColorBlendOp(vk::BlendOp::eAdd)
					.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
					.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
					.setAlphaBlendOp(vk::BlendOp::eAdd)
					.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
					.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
					.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
				},
		}));)
}

nest::Pipeline* nest::CreateGridPipeline()
{
	return BLEACH_NEW(Pipeline( s_kGridPipeline, nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreatePipeline(
	{
		.shaderPushConstants =
			{
				vk::PushConstantRange()
					.setOffset(0)
					.setSize(sizeof(PushConstants))
					.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
			},
			.shaderUniforms =
			{
				nest::CameraManager::s_cameraUniform,
			},
			.shaderStages =
			{
				vk::PipelineShaderStageCreateInfo()
					.setStage(vk::ShaderStageFlagBits::eVertex)
					.setPName(s_kMainName)
					.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule(s_kGridVert)),
				vk::PipelineShaderStageCreateInfo()
					.setStage(vk::ShaderStageFlagBits::eFragment)
					.setPName(s_kMainName)
					.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule(s_kGridFrag))
			},
			.polygonMode = vk::PolygonMode::eFill,
			.cullMode = vk::CullModeFlagBits::eNone,
			.frontFace = vk::FrontFace::eCounterClockwise,
			.depthTestEnable = true,
			.depthCompareOp = vk::CompareOp::eLess,
			.depthWriteEnable = false,
			.colorBlendStates = {
				vk::PipelineColorBlendAttachmentState()
					.setBlendEnable(true)
					.setColorBlendOp(vk::BlendOp::eAdd)
					.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
					.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
					.setAlphaBlendOp(vk::BlendOp::eAdd)
					.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
					.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
					.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
			},
	}) ));
}

nest::Pipeline* nest::CreateTexturedMeshPipeline()
{
	return BLEACH_NEW(Pipeline(s_kTexturedMeshPipeline, nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreatePipeline(
		{
			.vertexBindings =
			{
				vk::VertexInputBindingDescription()
					.setBinding(0)
					.setInputRate(vk::VertexInputRate::eVertex)
					.setStride(sizeof(nest::MeshVertex)),
			},
			.vertexAttributes =
			{
				vk::VertexInputAttributeDescription()
					.setLocation(0)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::position)),
				vk::VertexInputAttributeDescription()
					.setLocation(1)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::normal)),
				vk::VertexInputAttributeDescription()
					.setLocation(2)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::texcoord0)),
				vk::VertexInputAttributeDescription()
					.setLocation(3)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::tangent)),
				vk::VertexInputAttributeDescription()
					.setLocation(4)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::bitangent)),
			},
			.shaderPushConstants =
			{
				vk::PushConstantRange()
					.setOffset(0)
					.setSize(sizeof(PushConstants))
					.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
			},
			.shaderUniforms =
			{
				nest::CameraManager::s_cameraUniform,
				nest::LightManager::s_lightUniform,
				nest::LightManager::s_shadowMapUniform,
				nest::ResourceManager::s_diffuseUniform,
				nest::ResourceManager::s_normalUniform,
				nest::ResourceManager::s_roughnessUniform,
				nest::ResourceManager::s_metallicUniform,
				nest::ResourceManager::s_ambientOcclusion,
				nest::ResourceManager::s_opacityUniform,
				nest::SkyboxManager::s_skyboxUniform
			},
			.shaderStages =
			{
				vk::PipelineShaderStageCreateInfo()
					.setStage(vk::ShaderStageFlagBits::eVertex)
					.setPName(s_kMainName)
					.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule("textured.vert")),
				vk::PipelineShaderStageCreateInfo()
					.setStage(vk::ShaderStageFlagBits::eFragment)
					.setPName(s_kMainName)
					.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule("textured.frag"))
			},
			.polygonMode = s_polygonMode,
			.cullMode = s_cullMode,
			.frontFace = s_frontFace,
			.depthTestEnable = s_depthTest,
			.depthCompareOp = s_compareOp,
			.depthWriteEnable = s_depthWrite,
			.colorBlendStates =
			{
			vk::PipelineColorBlendAttachmentState()
				.setBlendEnable(true)
				.setColorBlendOp(vk::BlendOp::eAdd)
				.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setAlphaBlendOp(vk::BlendOp::eAdd)
				.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
			},
		}));)
}

void nest::RebuildTexturedMeshPipeline(nest::Pipeline* pipeline)
{
	auto vulkan = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan;
	vulkan.GetDevice().waitIdle();
	vulkan.DestroyPipeline(pipeline->pipeline);
	pipeline->pipeline = vulkan.CreatePipeline(
		{
			.vertexBindings =
			{
				vk::VertexInputBindingDescription()
					.setBinding(0)
					.setInputRate(vk::VertexInputRate::eVertex)
					.setStride(sizeof(nest::MeshVertex)),
			},
			.vertexAttributes =
			{
				vk::VertexInputAttributeDescription()
					.setLocation(0)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::position)),
				vk::VertexInputAttributeDescription()
					.setLocation(1)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::normal)),
				vk::VertexInputAttributeDescription()
					.setLocation(2)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::texcoord0)),
				vk::VertexInputAttributeDescription()
					.setLocation(3)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::tangent)),
						vk::VertexInputAttributeDescription()
					.setLocation(4)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::bitangent))
			},
			.shaderPushConstants =
			{
				vk::PushConstantRange()
					.setOffset(0)
					.setSize(sizeof(PushConstants))
					.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
			},
			.shaderUniforms =
			{
				nest::CameraManager::s_cameraUniform,
				nest::LightManager::s_lightUniform,
				nest::LightManager::s_shadowMapUniform,
				nest::ResourceManager::s_diffuseUniform,
				nest::ResourceManager::s_normalUniform,
				nest::ResourceManager::s_roughnessUniform,
				nest::ResourceManager::s_metallicUniform,
				nest::ResourceManager::s_ambientOcclusion,
				nest::ResourceManager::s_opacityUniform,
				nest::SkyboxManager::s_skyboxUniform
			},
			.shaderStages =
			{
				vk::PipelineShaderStageCreateInfo()
					.setStage(vk::ShaderStageFlagBits::eVertex)
					.setPName(s_kMainName)
					.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule("textured.vert")),
				vk::PipelineShaderStageCreateInfo()
					.setStage(vk::ShaderStageFlagBits::eFragment)
					.setPName(s_kMainName)
					.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule("textured.frag"))
			},
			.polygonMode = s_polygonMode,
			.cullMode = s_cullMode,
			.frontFace = s_frontFace,
			.depthTestEnable = s_depthTest,
			.depthCompareOp = s_compareOp,
			.depthWriteEnable = s_depthWrite,
			.colorBlendStates =
			{
			vk::PipelineColorBlendAttachmentState()
				.setBlendEnable(true)
				.setColorBlendOp(vk::BlendOp::eAdd)
				.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setAlphaBlendOp(vk::BlendOp::eAdd)
				.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
			},
		});
}

void nest::RebuildMeshPipeline(nest::Pipeline* pipeline)
{
	auto& vulkan = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan;
	vulkan.GetDevice().waitIdle();
	vulkan.DestroyPipeline(pipeline->pipeline);
	pipeline->pipeline = vulkan.CreatePipeline(
		{
				.vertexBindings =
				{
					vk::VertexInputBindingDescription()
						.setBinding(0)
						.setInputRate(vk::VertexInputRate::eVertex)
						.setStride(sizeof(nest::MeshVertex))
				},
				.vertexAttributes =
				{
					vk::VertexInputAttributeDescription()
						.setLocation(0)
						.setBinding(0)
						.setFormat(vk::Format::eR32G32B32Sfloat)
						.setOffset(offsetof(MeshVertex, MeshVertex::position)),
					vk::VertexInputAttributeDescription()
						.setLocation(1)
						.setBinding(0)
						.setFormat(vk::Format::eR32G32B32Sfloat)
						.setOffset(offsetof(MeshVertex, MeshVertex::normal)),
					vk::VertexInputAttributeDescription()
						.setLocation(2)
						.setBinding(0)
						.setFormat(vk::Format::eR32G32Sfloat)
						.setOffset(offsetof(MeshVertex, MeshVertex::tangent)),
					vk::VertexInputAttributeDescription()
						.setLocation(3)
						.setBinding(0)
						.setFormat(vk::Format::eR32G32Sfloat)
						.setOffset(offsetof(MeshVertex, MeshVertex::bitangent))
				},
				.shaderPushConstants =
				{
					vk::PushConstantRange()
						.setOffset(0)
						.setSize(sizeof(PushConstants))
						.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
				},
				.shaderUniforms =
				{
					nest::CameraManager::s_cameraUniform,
					nest::LightManager::s_lightUniform,
					nest::LightManager::s_shadowMapUniform,
				},
				.shaderStages =
				{
					vk::PipelineShaderStageCreateInfo()
						.setStage(vk::ShaderStageFlagBits::eVertex)
						.setPName(s_kMainName)
						.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule("mesh.vert")),
					vk::PipelineShaderStageCreateInfo()
						.setStage(vk::ShaderStageFlagBits::eFragment)
						.setPName(s_kMainName)
						.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule("mesh.frag"))
				},
				.polygonMode = s_polygonMode,
				.cullMode = s_cullMode,
				.frontFace = s_frontFace,
				.depthTestEnable = s_depthTest,
				.depthCompareOp = s_compareOp,
				.depthWriteEnable = s_depthWrite,
				.colorBlendStates =
				{
				vk::PipelineColorBlendAttachmentState()
					.setBlendEnable(true)
					.setColorBlendOp(vk::BlendOp::eAdd)
					.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
					.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
					.setAlphaBlendOp(vk::BlendOp::eAdd)
					.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
					.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
					.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
				},
		});
}

nest::Pipeline* nest::CreateShadowMapPipeline()
{
	return BLEACH_NEW(Pipeline(s_kShadowMapPipeline, nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreatePipeline(
		{
		.vertexBindings =
		{
			vk::VertexInputBindingDescription()
				.setBinding(0)
				.setInputRate(vk::VertexInputRate::eVertex)
				.setStride(sizeof(MeshVertex))
		},
		.vertexAttributes =
		{
			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(offsetof(MeshVertex, MeshVertex::position))
		},
		.shaderPushConstants =
		{
			vk::PushConstantRange()
				.setStageFlags(vk::ShaderStageFlagBits::eAllGraphics)
				.setOffset(0)
				.setSize(sizeof(ShadowPushConstants))
		},
		.shaderStages =
		{
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eVertex)
				.setPName(s_kMainName)
				.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule("shadow.vert")),
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eFragment)
				.setPName(s_kMainName)
				.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule("shadow.frag"))
		},
		.cullMode = vk::CullModeFlagBits::eFront,
		.frontFace = vk::FrontFace::eCounterClockwise,
		.depthTestEnable = true,
		.depthCompareOp = vk::CompareOp::eLessOrEqual,
		.depthWriteEnable = true,
		.depthBiasEnable = true,
		.colorBlendStates =
		{
			vk::PipelineColorBlendAttachmentState()
				.setBlendEnable(true)
				.setColorBlendOp(vk::BlendOp::eAdd)
				.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setAlphaBlendOp(vk::BlendOp::eAdd)
				.setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha)
				.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
				.setColorWriteMask(vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB)
		},
		.viewports =
		{
			vk::Viewport(0, 0, static_cast<float>(nest::LightManager::s_lightMapDimensions.width), static_cast<float>(nest::LightManager::s_lightMapDimensions.height), 0, 1)
		},
			.renderPass = nest::LightManager::s_targetShadowMap.renderPass,
		.dynamicStates =
		{
			vk::DynamicState::eDepthBias,
		}}));)
}

nest::Pipeline* nest::CreateSkyboxPipeline()
{
	return BLEACH_NEW(Pipeline(s_kSkyboxPipeline, nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreatePipeline(
		{
			.vertexBindings =
			{
				vk::VertexInputBindingDescription()
					.setBinding(0)
					.setInputRate(vk::VertexInputRate::eVertex)
					.setStride(sizeof(MeshVertex)),
			},
			.vertexAttributes =
			{
				vk::VertexInputAttributeDescription()
					.setLocation(0)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::position)),
				vk::VertexInputAttributeDescription()
					.setLocation(1)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32B32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::normal)),
				vk::VertexInputAttributeDescription()
					.setLocation(2)
					.setBinding(0)
					.setFormat(vk::Format::eR32G32Sfloat)
					.setOffset(offsetof(MeshVertex, MeshVertex::texcoord0)),
			},
			.shaderUniforms =
			{
				nest::CameraManager::s_cameraUniform,
				nest::SkyboxManager::s_skyboxUniform,
			},
			.shaderStages =
			{
				vk::PipelineShaderStageCreateInfo()
					.setStage(vk::ShaderStageFlagBits::eVertex)
					.setPName(s_kMainName)
					.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule(s_kSkyboxVert)),
				vk::PipelineShaderStageCreateInfo()
					.setStage(vk::ShaderStageFlagBits::eFragment)
					.setPName(s_kMainName)
					.setModule(nest::Engine::GetGraphics()->m_vulkanWrapper->shaderLib.GetModule(s_kSkyboxFrag))
			},
		})));
}

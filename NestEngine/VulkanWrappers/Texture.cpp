#include "Texture.h"
#include "../Managers/GraphicsSystem.h"
#include "../Configuration/LogMacro.hint"
#include "../Engine/Engine.h"
#include "../VulkanWrappers/Vulkan.h"
#include "../VulkanWrappers/Sampler.h"
#include "../VulkanWrappers/Image.h"
#include "../VulkanWrappers/ImageView.h"
#include "../VulkanWrappers/DescriptorSet.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../Managers/ResourceManager.h"
#include <SDL.h>
#include <SDL_image.h>
// Creates a texture at path of type flag
// Do this before main gameloop or during loading because it allocates 
// a sampler, image, image view, and set.
// Also, flag will set the specific descriptor set based on the type of texture
// this is. Only pass in the specific texture this type is as it is a bitwise check
// if you pass in multiple flags only the first found will be used.
nest::Texture::Texture(const std::string &path, Material::TextureFlags flag)
{
	SDL_Surface* color = LoadCompatibleImage(path);
	if (color == nullptr)
	{
		isValid = false;
	}
    sampler = BLEACH_NEW(Sampler{});
    image = BLEACH_NEW(Image{});
    imageView = BLEACH_NEW(ImageView{});
    set = BLEACH_NEW(DescriptorSet{});

	sampler->sampler = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().createSampler(
		vk::SamplerCreateInfo()
		.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
		.setMinFilter(vk::Filter::eLinear)
		.setMagFilter(vk::Filter::eLinear)
	
	);

	if (color != nullptr)
	{

		std::tie(image->image, imageView->view) = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateTexture2DImageAndView(
			{ static_cast<uint32_t>(color->w), static_cast<uint32_t>(color->h) },
			GAP311::VulkanWrapper::VulkanFormatFromSDLFormat(color->format->format),
			color->pixels, color->w * color->h * color->format->BytesPerPixel
		);
		isValid = true;
	}
	else
	{
		color = SDL_CreateRGBSurface(0, // Flags (0 for default behavior)
			1048, // Width in pixels
			1048, // Height in pixels
			32,  // Depth (bits per pixel, e.g., 32 for RGBA)
			0,   // Rmask (0 for default based on depth)
			0,   // Gmask (0 for default based on depth)
			0,   // Bmask (0 for default based on depth)
			0    // Amask (0 for default, results in Amask of 0)
		);
		SDL_Surface* newFormat = SDL_ConvertSurfaceFormat(color, SDL_PIXELFORMAT_ABGR8888, 0);
		if (newFormat)
		{
			SDL_FreeSurface(color);
			color = newFormat;
		}
		std::tie(image->image, imageView->view) = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateTexture2DImageAndView(
			{ static_cast<uint32_t>(color->w), static_cast<uint32_t>(color->h) },
			GAP311::VulkanWrapper::VulkanFormatFromSDLFormat(color->format->format),
			color->pixels, color->w* color->h* color->format->BytesPerPixel);
	}
	if ((flag & Material::eDiffuse) != 0)
		set->set = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateAlternateImageValueForUniform(nest::ResourceManager::s_diffuseUniform, imageView->view, sampler->sampler);
	else if ((flag & Material::eNormal) != 0)
		set->set = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateAlternateImageValueForUniform(nest::ResourceManager::s_normalUniform, imageView->view, sampler->sampler);
	else if ((flag & Material::eRoughness) != 0)
		set->set = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateAlternateImageValueForUniform(nest::ResourceManager::s_roughnessUniform, imageView->view, sampler->sampler);
	else if ((flag & Material::eMetallic) != 0)
		set->set = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateAlternateImageValueForUniform(nest::ResourceManager::s_metallicUniform, imageView->view, sampler->sampler);
	else if ((flag & Material::eAO) != 0)
		set->set = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateAlternateImageValueForUniform(nest::ResourceManager::s_ambientOcclusion, imageView->view, sampler->sampler);
	else if ((flag & Material::eOpacity) != 0)
		set->set = nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateAlternateImageValueForUniform(nest::ResourceManager::s_opacityUniform, imageView->view, sampler->sampler);
}

nest::Texture::Texture()
	: set(BLEACH_NEW(DescriptorSet{}))
	, image(BLEACH_NEW(Image{}))
	, imageView(BLEACH_NEW(ImageView{}))
	, sampler(BLEACH_NEW(Sampler{}))
{}

SDL_Surface* nest::Texture::LoadCompatibleImage(std::filesystem::path imageFile, uint32_t pixelFormat)
{
	if (pixelFormat == 0)
		pixelFormat = SDL_PIXELFORMAT_ABGR8888;
	SDL_Surface* surface = IMG_Load(imageFile.string().c_str());
	if (!surface)
	{
		return nullptr;
	}

	if (surface->format->format != pixelFormat)
	{
		SDL_Surface* newFormat = SDL_ConvertSurfaceFormat(surface, pixelFormat, 0);
		if (newFormat)
		{
			SDL_FreeSurface(surface);
			surface = newFormat;
		}
	}
	return surface;
}

nest::Texture::~Texture()
{
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().destroySampler(sampler->sampler);
	nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyImageAndView(image->image, imageView->view);
    nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.FreeDescriptorSet(set->set);
    BLEACH_DELETE(sampler);
    BLEACH_DELETE(image);
    BLEACH_DELETE(imageView);
    BLEACH_DELETE(set);
}
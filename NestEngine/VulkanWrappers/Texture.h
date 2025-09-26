#pragma once
#include "../Uniforms/MeshDataStructs.h"
#include <cstdint>
#include <filesystem>
struct SDL_Surface;
struct aiMaterial;
namespace nest
{
	struct DescriptorSet;
	struct Image;
	struct ImageView;
	struct Sampler;
	// Texture stores the descriptor set and allocated image data for loaded textures
	struct Texture
	{
		DescriptorSet* set;
		Image* image;
		ImageView* imageView;
		Sampler* sampler;
		bool isValid = false;
		Texture(const std::string& path, Material::TextureFlags flag);
		Texture();
		static SDL_Surface* LoadCompatibleImage(std::filesystem::path imageFile, uint32_t pixelFormat = 0);
		~Texture();
	};
}
#pragma once
#include "../Managers/Manager.h"
#include "../Uniforms/MeshDataStructs.h"
#include "../PBRTextureStack.h"
#include <unordered_map>
struct aiMaterial;
namespace nest
{	
	struct Texture;
	struct TextureData;
	class ResourceManager : public Manager
	{
		// tracks total count of textures
		static inline int s_textureCounter = 0;
		static inline const int s_kInvalidIndex = -1;
		std::unordered_map<int, Material> m_materialMap{};
		std::unordered_map<int, PBRTextureStack*> m_textures{};
	public:
		static GAP311::ShaderUniform s_diffuseUniform;
		static GAP311::ShaderUniform s_normalUniform;
		static GAP311::ShaderUniform s_roughnessUniform;
		static GAP311::ShaderUniform s_metallicUniform;
		static GAP311::ShaderUniform s_ambientOcclusion;
		static GAP311::ShaderUniform s_opacityUniform;
	public:
		SET_HASHED_ID(ResourceManager)
		// creates a texture and stores it into the resource managers map. 
		// Returns ID of texture to be used as an index later
		bool Init() override;
		void Exit() override;
		~ResourceManager() override = default;
		int AddTexture(const std::string& path, aiMaterial* material);
		PBRTextureStack* GetTexture(int index);
		void AddMaterial(int index, Material mat);
		Material GetMaterial(int index) { return m_materialMap[index]; }
	};
}
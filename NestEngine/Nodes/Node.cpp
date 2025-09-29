#include "../Configuration/LogMacro.hint"
#include "../Engine/Engine.h"
#include "../GlobalConstants/constants.h"  
#include "../lib/BleachNew/BleachNew.h"
#include "../Managers/CameraManager.h"
#include "../Managers/GraphicsSystem.h"
#include "../Managers/LightManager.h"
#include "../Managers/ResourceManager.h"
#include "../Managers/SkyboxManager.h"
#include "../Math/MathAlias.h"
#include "../Nodes/Node.h"
#include "../Uniforms/MeshDataStructs.h"
#include "../VulkanWrappers/DescriptorSet.h"
#include "../VulkanWrappers/Pipeline.h"
#include "../VulkanWrappers/Texture.h"
#include "../VulkanWrappers/Vulkan.h"
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
nest::ParentNode::~ParentNode()
{
    for (auto& child : m_children)
    {
        BLEACH_DELETE(child);
    }
}

void nest::MeshNode::RenderRelative(const Pipeline* pipeline, vk::CommandBuffer& commands) const
{
    PushConstants constants;
    auto combinedTransform = static_cast<MeshNode*>(m_parent)->m_meshData.transform * m_meshData.transform;
    constants.objectMatrix = combinedTransform;
    constants.objectMaterial = m_meshData.material;
    commands.pushConstants(pipeline->pipeline->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(constants), &constants);
    BindToPipeline(pipeline, commands);
    commands.drawIndexed(static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
}

void nest::MeshNode::RenderWorld(const Pipeline* pipeline, vk::CommandBuffer& commands) const
{
    auto camUniform = nest::Engine::GetManager<CameraManager>()->GetActiveCameraUniform();
    PushConstants constants;
    constants.objectMatrix = m_meshData.transform;
    constants.objectMaterial = m_meshData.material;
    commands.pushConstants(pipeline->pipeline->GetLayout(), vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(constants), &constants);
    BindToPipeline(pipeline, commands);
    commands.drawIndexed(static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
 }

Mat4 nest::MeshNode::GetTransform() const
{
    if (m_parent == nullptr)
        return m_meshData.transform;
    else
        return static_cast<MeshNode*>(m_parent)->m_meshData.transform * m_meshData.transform;
}

Vec3 nest::MeshNode::GetPos() const
{
    auto transform = GetTransform();
    return { transform[3][0], transform[3][1], transform[3][2] };
}

void nest::MeshNode::Draw(vk::CommandBuffer& commands)
{
    commands.bindVertexBuffers(0, { m_vertexBuffer->GetBuffer() }, {0});
    commands.bindIndexBuffer(m_indexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
    commands.drawIndexed(m_indexCount, 1, 0, 0, 0);
}

void nest::MeshNode::BindToPipeline(const Pipeline* pipeline, vk::CommandBuffer& commands) const
{
    if (m_meshData.material.hasTexture)
    {
        auto stack = nest::Engine::GetManager<nest::ResourceManager>()->GetTexture(m_meshData.material.textureId);
        auto diffuseSet = (m_meshData.material.textureFlags & Material::eDiffuse) ? stack->GetTexture(Material::eDiffuse)->set->set : nullptr;
        auto normalSet = (m_meshData.material.textureFlags & Material::eNormal) ? stack->GetTexture(Material::eNormal)->set->set : nullptr;
        auto roughnessSet = (m_meshData.material.textureFlags & Material::eRoughness) ? stack->GetTexture(Material::eRoughness)->set->set : nullptr;
        auto metallicSet = (m_meshData.material.textureFlags & Material::eMetallic) ? stack->GetTexture(Material::eMetallic)->set->set : nullptr;
        auto aoSet = stack->GetTexture(Material::eAO)->set->set;
        auto opacitySet = stack->GetTexture(Material::eOpacity)->set->set;
        commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->pipeline->GetLayout(), 0, { nest::CameraManager::s_cameraUniform, nest::LightManager::s_lightUniform, nest::LightManager::s_shadowMapUniform,
            diffuseSet,
            normalSet,
            roughnessSet,
            metallicSet,
            aoSet,
            opacitySet,
            SkyboxManager::s_skyboxUniform }, {});
    }
    else
    {
        commands.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline->pipeline->GetLayout(), 0, { nest::CameraManager::s_cameraUniform, nest::LightManager::s_lightUniform, nest::LightManager::s_shadowMapUniform }, { });
    }
    commands.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->pipeline->GetPipeline());
    commands.bindIndexBuffer(m_indexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);
    commands.bindVertexBuffers(0, { m_vertexBuffer->GetBuffer() }, { 0 });
}

nest::MeshNode::MeshNode(const aiMesh* mesh)
{
    m_meshData.material = nest::Engine::GetManager<nest::ResourceManager>()->GetMaterial(mesh->mMaterialIndex);
    if (m_meshData.material.hasTexture)
    {
        m_pipelineId = s_kTexturedMeshPipeline;
    }
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        Vec2 defaultTexCoords = { 0.f, 0.f};
        if (mesh->HasNormals())
        {
            const auto& normal = mesh->mNormals[i];
            Vec3 xyz = Vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            // Check if there are any texture coordinates at all
            if (mesh->GetNumUVChannels() == 0 || mesh->mTextureCoords[0] == nullptr)
            {
                _LOG(LogType::kError, "No texture coordinates found, using defaults");
                m_vertices.push_back({ xyz, {-normal.x, -normal.y, -normal.z}, defaultTexCoords });
            }
            else
            {
                aiVector3D& texCoord = mesh->mTextureCoords[0][i];

                // Check for weird values
                if (std::isnan(texCoord.x) || std::isnan(texCoord.y))
                {
                    _LOG(LogType::kError, "Invalid UVs detected, using defaults");
                    texCoord.x = texCoord.y = 0.f;
                }

                // Use only x,y (z is for 3D textures, which are rare)
                Vec2 uv = { texCoord.x, texCoord.y };
                m_vertices.push_back({ xyz, {-normal.x, -normal.y, -normal.z}, uv });
            }
        }
        else
        {
            Vec3 xyz = Vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            m_vertices.push_back({ xyz, -glm::normalize(xyz), defaultTexCoords });
        }
    }
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        const auto& face = mesh->mFaces[i];

        for (uint32_t ii = 0; ii < face.mNumIndices; ++ii)
        {
            AddIndex(face.mIndices[ii]);
        }
    }
    BuildBuffers();
}

nest::MeshNode::~MeshNode()
{
    nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().waitIdle();
    nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyBuffer(m_vertexBuffer->GetBuffer());
    nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.DestroyBuffer(m_indexBuffer->GetBuffer());
    BLEACH_DELETE(m_indexBuffer);
    BLEACH_DELETE(m_vertexBuffer);
    ParentNode::~ParentNode();
}

void nest::MeshNode::BuildBuffers()
{
    m_indexBuffer = BLEACH_NEW(RenderBuffer);
    m_vertexBuffer = BLEACH_NEW(RenderBuffer);
    m_indexBuffer->SetBuffer(nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateIndexBuffer(sizeof(m_indices[0]) * m_indices.size(), m_indices.data()));
    m_vertexBuffer->SetBuffer(nest::Engine::GetGraphics()->m_vulkanWrapper->vulkan.CreateBuffer(vk::BufferUsageFlagBits::eVertexBuffer, sizeof(m_vertices[0]) * m_vertices.size(), m_vertices.data()));
}

void nest::MeshNode::BuildTangents()
{
    for (int i = 0; i < m_vertices.size(); i += 3)
    {
        // Shortcuts for vertices
        glm::vec3& v0 = m_vertices[i + 0].position;
        glm::vec3& v1 = m_vertices[i + 1].position;
        glm::vec3& v2 = m_vertices[i + 2].position;

        // Shortcuts for UVs
        glm::vec2& uv0 = m_vertices[i + 0].texcoord0;
        glm::vec2& uv1 = m_vertices[i + 1].texcoord0;
        glm::vec2& uv2 = m_vertices[i + 2].texcoord0;

        // Edges of the triangle : position delta
        glm::vec3 deltaPos1 = v1 - v0;
        glm::vec3 deltaPos2 = v2 - v0;

        // UV delta
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;
        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;
        m_vertices[i + 0].tangent = tangent;
        m_vertices[i + 1].tangent = tangent;
        m_vertices[i + 2].tangent = tangent;

        m_vertices[i + 0].bitangent = bitangent;
        m_vertices[i + 1].bitangent = bitangent;
        m_vertices[i + 2].bitangent = bitangent;
    }
}

void nest::MeshNode::QueRender()
{
    nest::Engine::GetGraphics()->QueRender(this);
    for (auto& child : m_children)
        child->QueRender();
}

void nest::MeshNode::Render(vk::CommandBuffer& commands) const
{
    if (m_parent == nullptr)
        RenderWorld(nest::Engine::GetGraphics()->GetPipeline(m_pipelineId), commands);
    else
        RenderRelative(nest::Engine::GetGraphics()->GetPipeline(m_pipelineId), commands);
}

nest::SceneNode::SceneNode(const aiNode* aiNode)
{
    m_meshData.name = aiNode->mName.C_Str();
}

void nest::SceneNode::QueRender()
{
    for (auto& child : m_children)
        child->QueRender();
}


// note to future self, make this work and were golden pony boy
void nest::BaseNode::SetMeshData(const aiNode* node) 
{
    m_meshData.name = node->mName.C_Str();
    m_id = Hash(m_meshData.name.c_str());
    auto& t = node->mTransformation; // already left-handed

    // glm uses column-major whereas Assimp uses row-major
    // https://stackoverflow.com/questions/33862730/row-major-vs-column-major-confusion
	// so we need to transpose the matrix to get it into the correct format
    // GLM matrices are made of vectors, where each vector represents a column
	// whereas Mat4 is made of arrays, where the first index is the column, and second index is the row
    m_meshData.transform = glm::transpose(glm::make_mat4(&t.a1));
    if (strcmp(m_meshData.name.c_str(), "Camera") == 0)
    {
        nest::Engine::GetManager<CameraManager>()->SetCameraTransform(m_meshData.transform);
    }

#if _PRINT_NEW_TRANSFORMS == 1
    std::cout << node->mName.C_Str() << " = \n";
    std::cout << m_meshData.transform << std::endl;
#endif
}

bool nest::BaseNode::IsOpaque() const
{
    return (m_meshData.material.color.w >= 1.f && m_meshData.material.hasTransparency == false);
}


void nest::ParentNode::Render(vk::CommandBuffer& commands) const
{
    for (auto& child : m_children)
        child->Render(commands);
}

nest::BaseNode* nest::ParentNode::FindNode(HashedId id)
{
    for (auto& child : m_children)
    {
        if (child->GetId() == id)
            return child;
    }
    return nullptr;
}

nest::LightNode::LightNode(const aiLight* aiLight, const aiNode* aiNode)
{
    BaseNode::SetMeshData(aiNode);
    m_lightData.diffuse = Vec3{ aiLight->mColorDiffuse.r, aiLight->mColorDiffuse.g, aiLight->mColorDiffuse.b };
    m_lightData.specular = Vec3{ aiLight->mColorSpecular.r, aiLight->mColorSpecular.g , aiLight->mColorSpecular.b };
    m_lightData.ambient = Vec3{ aiLight->mColorAmbient.r, aiLight->mColorAmbient.g , aiLight->mColorAmbient.b };
    m_lightData.position = m_meshData.transform[3];
    m_lightData.size = { aiLight->mSize.x, aiLight->mSize.y };
    m_lightData.up = { aiLight->mUp.x, aiLight->mUp.y, aiLight->mUp.z };
    m_lightData.direction = { -aiLight->mDirection.z, aiLight->mDirection.y, aiLight->mDirection.x };
    m_lightData.attenuation = { aiLight->mAttenuationConstant, aiLight->mAttenuationLinear, aiLight->mAttenuationQuadratic };
    m_lightData.innerCone = glm::degrees(aiLight->mAngleInnerCone);
    m_lightData.outerCone = glm::degrees(aiLight->mAngleOuterCone);

    m_lightData.cutoff = glm::cos(glm::radians(m_lightData.outerCone)); // should already be in radians
    switch (aiLight->mType)
    {
    case aiLightSourceType::aiLightSource_POINT:
        m_lightData.type = Light::ePoint;
        break;
    case aiLightSourceType::aiLightSource_DIRECTIONAL:
        m_lightData.type = Light::eSun;
        break;
    case aiLightSourceType::aiLightSource_SPOT:
        m_lightData.type = Light::eSpot;
        break;
    case aiLightSourceType::aiLightSource_AREA:
        m_lightData.type = Light::eArea;
        break;
    default:
        m_lightData.type = Light::ePoint;
        break;
    }
   
}

void nest::LightNode::QueRender()
{

}

nest::CameraNode::CameraNode([[maybe_unused]] const aiNode* aiNode)
{
    //
}

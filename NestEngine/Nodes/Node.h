#pragma once
#include "../Configuration/Macros.h"
#include "../DataStructs/TransformData.h"
#include "../GlobalConstants/constants.h"
#include "../RenderBuffer.h"
#include "../Math/MathAlias.h"
#include <vector>
namespace vk
{
	class Buffer;
	class CommandBuffer;
}
struct aiMesh;
struct aiNode;
struct aiLight;
namespace nest
{
	struct Pipeline;
	class CameraObject;
	class BaseNode
	{
	protected:
		HashedId m_id = 0;
		MeshData m_meshData;
	public:
		SET_HASHED_ID(BaseNode)
		BaseNode() = default;
		virtual ~BaseNode() = default;
		virtual void QueRender() {};
		virtual void Render([[maybe_unused]] vk::CommandBuffer& commands) const {};
		MeshData& GetTransformData() { return m_meshData; }
		MeshData GetTransformDataC() const { return m_meshData; }
		void SetMeshData(const aiNode* node);
		bool IsOpaque() const;
		HashedId GetId() const { return m_id; }
	};

	class ParentNode : public BaseNode
	{
	protected:
		std::vector<BaseNode*> m_children{};
	public:
		SET_HASHED_ID(BaseNode)
		ParentNode() = default;
		virtual ~ParentNode() override;
		virtual void EmplaceNode([[maybe_unused]] BaseNode* node) { m_children.push_back(node); }
		virtual std::vector<BaseNode*> GetChildren() { return m_children; }
		virtual void Render(vk::CommandBuffer& commands) const override;
		BaseNode* FindNode(HashedId id);
	};
	
	// Mesh nodes contain all the data to draw a single mesh
	class MeshNode : public ParentNode
	{
	public:
		MeshNode* m_parent = nullptr;
		std::vector<nest::MeshVertex> m_vertices;
		std::vector<uint32_t> m_indices;
		RenderBuffer* m_indexBuffer;
		RenderBuffer* m_vertexBuffer;
		uint32_t m_indexCount = 0;
		HashedId m_pipelineId = s_kMeshPipeline;
	public:
		SET_HASHED_ID(MeshNode)
		MeshNode(const aiMesh* mesh);
		MeshNode() = default;
		~MeshNode() override;
		void SetOwner([[maybe_unused]] MeshNode* node) { m_parent = node; }
		void AddVertex(nest::MeshVertex vertex) { m_vertices.emplace_back(vertex); }
		void AddIndex(uint32_t index) { m_indices.emplace_back(index); ++m_indexCount; }
		void BuildBuffers();
		void QueRender() override;
		void Render([[maybe_unused]] vk::CommandBuffer& commands) const override;
		void RenderRelative(const Pipeline* pipeline, [[maybe_unused]] vk::CommandBuffer& commands)  const;
		void RenderWorld(const Pipeline* pipeline, [[maybe_unused]] vk::CommandBuffer& commands) const;
		HashedId GetPipelineId() const { return m_pipelineId; }
		Mat4 GetTransform() const;
		Vec3 GetPos() const;
		void Draw(vk::CommandBuffer& commands);
	private:
		void BindToPipeline(const Pipeline* pipeline, [[maybe_unused]] vk::CommandBuffer& commands) const;
	};

	class SceneNode : public ParentNode
	{
	public:
		SET_HASHED_ID(SceneNode)
		SceneNode() = default;
		SceneNode(const aiNode* aiNode);
		void QueRender() override;
	};

	class LightNode : public BaseNode
	{
		Light m_lightData{};
	public:
		SET_HASHED_ID(LightNode)
		LightNode() = default;
		LightNode(const aiLight* aiLight, const aiNode* aiNode);
		void QueRender() override;
		Light GetData() { return m_lightData; }
		void SetData(Light data) { m_lightData = data; }
	};

	class CameraNode : public BaseNode
	{
		CameraObject* m_camera = nullptr;
	public:
		SET_HASHED_ID(CameraNode)
		CameraNode() = default;
		CameraNode(const aiNode* aiNode);

	};
}
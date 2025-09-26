#include "ShapeFactory.h"
#include "../Nodes/Node.h"
#include "../Engine/Engine.h"
#include "../Managers/GraphicsSystem.h"
#include "../VulkanWrappers/Vulkan.h"
nest::Result nest::ShapeFactory::LoadCube(MeshNode* mesh)
{
	//
	//   4----5
	//  /|   /|
	// 0----1 |
	// | 7--|-6    y z
	// |/   |/     |/
	// 3----2      +--x
	//
	//mesh = BLEACH_NEW(MeshNode);
	MeshVertex vertices[] =
	{
		{ { -0.5f,  0.5f, -0.5f }, {}, { } },
		{ {  0.5f,  0.5f, -0.5f }, {}, { } },
		{ {  0.5f, -0.5f, -0.5f }, {}, { } },
		{ { -0.5f, -0.5f, -0.5f }, {}, { } },
		{ { -0.5f,  0.5f,  0.5f }, {}, { } },
		{ {  0.5f,  0.5f,  0.5f }, {}, { } },
		{ {  0.5f, -0.5f,  0.5f }, {}, { } },
		{ { -0.5f, -0.5f,  0.5f }, {}, { } },
	};

	for (auto& vertex : vertices)
	{
		vertex.normal = glm::normalize(vertex.position);
		vertex.texcoord0.x = vertex.position.x + 0.5f;
		vertex.texcoord0.y = vertex.position.y + 0.5f;
		mesh->AddVertex(vertex);
	}

	const uint32_t indices[] =
	{
		0, 3, 2, // Front
		0, 2, 1,
		4, 5, 7, // Rear
		5, 6, 7,
		1, 2, 6, // Right
		5, 1, 6,
		0, 4, 7, // Left
		0, 7, 3,
		5, 4, 0, // Top
		5, 0, 1,
		7, 6, 2, // Bottom
		7, 2, 3,
	};
	for (uint32_t indice : indices)
		mesh->AddIndex(indice);
	mesh->BuildBuffers();
    return Result::kSucess;
}

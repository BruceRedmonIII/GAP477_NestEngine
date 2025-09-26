#pragma once
#include "../Math/MathAlias.h"
#include "../HelperFiles/IdTypes.h"
#include <memory>

namespace GAP311
{
	struct GraphicsPipeline;
}
namespace nest
{
	struct Pipeline
	{
		HashedId id = 0;
		std::shared_ptr<GAP311::GraphicsPipeline> pipeline = nullptr;
		Pipeline() = default;
		Pipeline(const HashedId id, std::shared_ptr<GAP311::GraphicsPipeline> pipeline);
		~Pipeline() = default;
		Pipeline(const Pipeline& copy) = delete;
		Pipeline& operator=(const Pipeline& copy) = delete;
		Pipeline(const Pipeline&& copy) = delete;
		Pipeline& operator=(const Pipeline&& copy) = delete;
	};
}
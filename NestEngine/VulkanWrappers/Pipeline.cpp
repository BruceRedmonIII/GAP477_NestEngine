#include "Pipeline.h"
#include "../HelperFiles/IdTypes.h"
#include "../lib/gap-311/VulkanWrapper.hpp"
nest::Pipeline::Pipeline(const HashedId id, std::shared_ptr<GAP311::GraphicsPipeline> pipeline)
	: id(id)
	, pipeline(pipeline)
{}
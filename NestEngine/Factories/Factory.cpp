#include "Factory.h"

nest::Result nest::Factory::CreateComponent([[maybe_unused]] HashedId componentId, [[maybe_unused]] nest::GameObject* owner, [[maybe_unused]] const nest::XMLNode& dataFile)
{ 
    return nest::Result::kFailedToCreate;
}
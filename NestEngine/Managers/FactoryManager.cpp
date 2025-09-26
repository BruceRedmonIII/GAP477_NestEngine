#include "../Factories/Factory.h"
#include "FactoryManager.h"
#include "../Objects/GameObject.h"
#include "../lib/BleachNew/BleachNew.h"
#include <ranges>

nest::FactoryManager::~FactoryManager()
{
    BLEACH_DELETE(m_pFactories);
}
bool nest::FactoryManager::Init()
{
    m_pFactories = BLEACH_NEW(Factories);
#if _MANAGER_INIT_LOGGING == 1
    _LOG(LogType::kLog, "FactoryManager init succesful!");
#endif
    return true;
}

nest::Result nest::FactoryManager::CreateComponentById(HashedId id, nest::GameObject* owner, const nest::XMLNode& dataFile) const
{
    const auto& factories = m_pFactories->GetAllComponents();
    for (const auto& factory : std::views::values(factories))
    {
        nest::Result returnedValue = factory->CreateComponent(id, owner, dataFile);
        switch (returnedValue)
        {
            case nest::Result::kFailedToFind:
            {
                continue;
            }
            case nest::Result::kSucess:
            {
                return returnedValue;
            }
            case nest::Result::kFailedToCreate:
            {
                _LOG_V(LogType::kError, "Component failed to create! id = ", id);
                return returnedValue;
            }
        }
        _LOG_V(LogType::kError, "Component failed to find! id = ", id);
        return nest::Result::kFailedToFind;
    }
    return nest::Result::kFailedToFind;
}

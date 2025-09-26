#pragma once
#include "../Factories/Factory.h"
#include "../Objects/GameObject.h"
#include "../DataWrappers/HashMapWrapper.h"
#include "Manager.h"
namespace nest
{
    class XMLNode;
    class Factory;
    class FactoryManager : public Manager
    {
        using Factories = HashMapWrapper<Factory>;
        Factories* m_pFactories = nullptr;
    public:
        SET_HASHED_ID(FactoryManager)
        FactoryManager() = default;
        ~FactoryManager() override;
        FactoryManager(const FactoryManager& copy) = delete;
        FactoryManager& operator=(const FactoryManager& copy) = delete;
        bool Init();
        Result CreateComponentById(HashedId id, nest::GameObject* owner, const nest::XMLNode& dataFile) const;

        template <class FactoryType>
        void RegisterFactory(FactoryType* factory)
        {
            m_pFactories->Emplace(factory);
        }
    };
};

#pragma once
#include "../Configuration/LogMacro.hint"
#include "../lib/BleachNew/BleachNew.h"
#include "../Math/Hash.h"
#include <memory>
#include <type_traits>
#include <unordered_map>

namespace nest
{
    template <class DerivedType>
    concept HashedAbstract = requires
    {
        std::is_abstract_v<DerivedType>&& DerivedType::kHashedId;
    };

    template <class DerivedType>
    concept Derived = requires
    {
        std::is_abstract_v<DerivedType>;
    };

    template <class WrapType>
    concept Polymorphic = requires
    {
        std::is_polymorphic_v<WrapType>;
    };

    template <Polymorphic WrapType>
    class HashMapWrapper
    {
        std::unordered_map<HashedId, WrapType*> m_pContainer{};
    public:
        HashMapWrapper() = default;
        ~HashMapWrapper()
        {
            for (auto& comp : m_pContainer)
            {
                BLEACH_DELETE(comp.second);
            }
        }
        HashMapWrapper(const HashMapWrapper& copy) = delete;
        HashMapWrapper& operator=(const HashMapWrapper& copy) = delete;

        template <HashedAbstract DerivedType>
        bool Emplace(DerivedType* component)
        {
            static_assert(std::is_base_of_v<WrapType, DerivedType>, "Must be derived from the WrapType!");
            auto pair = m_pContainer.try_emplace(DerivedType::kHashedId, component);
            if (!pair.second)
            {
                _LOG(LogType::kError, "Derived type failed to emplace!");
                return false;
            }
            return true;
        }

        // Gather component from game object and cast to the corret type
        template <HashedAbstract DerivedType>
        DerivedType* GetContained()
        {
            static_assert(std::is_base_of_v<WrapType, DerivedType>, "Must be derived from the WrapType!");
            auto findIt = m_pContainer.find(DerivedType::kHashedId);
            if (findIt != m_pContainer.cend())
            {
                return static_cast<DerivedType*>(findIt->second);
            }
            _LOG(LogType::kError, "Contained type not found to retrieve!");
            return nullptr;
        }

        // Check to see if the ID exists using the DerivedTypes hashed id
        template <HashedAbstract DerivedType>
        bool DoesContain()
        {
            auto findIt = m_pContainer.find(DerivedType::kHashedId);
            if (findIt != m_pContainer.cend())
            {
                return true;
            }
            return false;
        }

        // Check to see if the ID exists
        bool DoesContain(HashedId id)
        {
            auto findIt = m_pContainer.find(id);
            if (findIt != m_pContainer.cend())
            {
                return true;
            }
            return false;
        }

        // Gather component from game object and cast to the corret type
        template <HashedAbstract DerivedType>
        void RemoveContained()
        {
            static_assert(std::is_base_of_v<WrapType, DerivedType>, "Must be derived from the WrapType!");
            auto findIt = m_pContainer.find(DerivedType::kHashedId);
            if (findIt != m_pContainer.cend())
            {
                m_pContainer.erase(findIt);
                return;
            }
            _LOG(LogType::kError, "Id not found to remove!");
        }

        // Removes the component from the container and deletes it
        // then sets to nullptr
        template <HashedAbstract DerivedType>
        void DestroyContained()
        {
            static_assert(std::is_base_of_v<WrapType, DerivedType>, "Must be derived from the WrapType!");
            auto findIt = m_pContainer.find(DerivedType::kHashedId);
            if (findIt != m_pContainer.cend())
            {
                BLEACH_DELETE(findIt->second);
                m_pContainer.erase(findIt);
                return;
            }
            _LOG(LogType::kError, "Id not found to destroy!");
        }

        // Gather component from game object and cast to the corret type
        void EraseById(HashedId id)
        {
            auto findIt = m_pContainer.find(id);
            if (findIt != m_pContainer.cend())
            {
                m_pContainer.erase(findIt);
                return;
            }
            _LOG(LogType::kError, "ID not found to remove! id = " + id);
        }


        // map component with label to this object
        template <Derived DerivedType>
        void EmplaceById(HashedId id, DerivedType* component)
        {
            auto pair = m_pContainer.try_emplace(id, component);
            if (!pair.second)
            {
                _LOG(LogType::kError, "Derived type failed to emplace! id = " + id);
            }
        }

        // Gather component from game object and cast to the corret type
        template <Derived DerivedType>
        DerivedType* GetContainedById(HashedId id)
        {
            auto findIt = m_pContainer.find(id);
            if (findIt != m_pContainer.cend())
            {
                return static_cast<DerivedType*>(findIt->second);
            }
            _LOG(LogType::kError, "Contained type not found to retrieve! id = " + id);
            return nullptr;
        }

        // Gather component from game object and cast to the corret type
        WrapType* GetById(HashedId id)
        {
            auto findIt = m_pContainer.find(id);
            if (findIt != m_pContainer.cend())
            {
                return (findIt->second);
            }
            _LOG(LogType::kError, "Contained type not found to retrieve! id = " + id);
            return nullptr;
        }
        std::unordered_map<HashedId, WrapType*>& GetAllComponents() { return m_pContainer; }
    };
}
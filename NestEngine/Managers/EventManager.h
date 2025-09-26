#pragma once
#include "Manager.h"
#include "../Configuration/Macros.h"
#include "../Events/Event.h"
#include <queue>
#include <unordered_map>


namespace nest
{
    class BaseObject;
    class MessagingComponent;
    class UIContainer;
    class UIWidget;
    class EventManager : public Manager
    {
        //TODO: convert to data buffers
        std::queue<Event*> m_messageQue;
        std::unordered_multimap<HashedId, BaseObject*> m_objectListeners;
        std::unordered_multimap<HashedId, Manager*> m_managerListeners;
    public:
        SET_HASHED_ID(EventManager)
        EventManager();
        ~EventManager() override;
        EventManager(const EventManager& copy) = delete;
        EventManager& operator=(const EventManager& copy) = delete;
        void RegisterObjectListener(HashedId evt, BaseObject* listener);
        void UnregisterObjectListener(HashedId evt, const BaseObject* listener);

        void RegisterManagerListener(HashedId evt, Manager* listener);
        void UnregisterManagerListener(HashedId evt, const Manager* listener);

        void QueEvent(Event* evt);
        void Update([[maybe_unused]] float deltaTime) override;
    };
}

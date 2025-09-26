#include "EventManager.h"
#include "../Engine/Engine.h"
#include "../Objects/BaseObject.h"
#include "../GlobalConstants/constants.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../Events/EventIdHashes.h"

nest::EventManager::EventManager()
    : m_objectListeners({})
{
#if _MANAGER_INIT_LOGGING == 1
    _LOG(LogType::kLog, "EventManager init succesful!");
#endif
}

nest::EventManager::~EventManager()
{
    while (!m_messageQue.empty())
    {
        BLEACH_DELETE(m_messageQue.front());
        m_messageQue.pop();
    }
}

void nest::EventManager::RegisterObjectListener(HashedId evt, BaseObject* listener)
{
    m_objectListeners.emplace(evt, listener);
}

void nest::EventManager::UnregisterObjectListener([[maybe_unused]] HashedId evt, [[maybe_unused]] const BaseObject* listener)
{
    //size_t amount = m_objectListeners.count(&evt);
    //auto it = m_objectListeners.find(&evt); // finds the first message
    //for (size_t i = 0; i < amount; ++i)
    //{
    //    if (it->second == listener)
    //    {
    //        m_objectListeners.erase(it);
    //        return;
    //    }
    //}
}

void nest::EventManager::RegisterManagerListener(HashedId evt, Manager* listener)
{
    m_managerListeners.emplace(evt, listener);
}

void nest::EventManager::UnregisterManagerListener([[maybe_unused]] HashedId evt, [[maybe_unused]] const Manager* listener)
{
    //m_managerListeners.emplace(evt, listener);
}

void nest::EventManager::QueEvent(Event* evt)
{
    if (evt->GetId() == kQuitGame)
    {
        nest::Engine::GetInstance()->Quit(true);
    }
    else
        m_messageQue.emplace(evt);
}

void nest::EventManager::Update([[maybe_unused]] float deltaTime)
{
    std::queue<Event*> buffQue;
    buffQue.swap(m_messageQue);
    while (!buffQue.empty())
    {
        auto objectIterators = m_objectListeners.equal_range(buffQue.front()->GetId());
        auto managerListeners = m_managerListeners.equal_range(buffQue.front()->GetId());

#if _LOG_EVENTS == 1
        _LOG_V(LogType::kLog, "Sending Event: ", buffQue.front()->GetId()));
#endif
        for (auto& it = objectIterators.first; it != objectIterators.second; ++it)
        {
            it->second->HandleEvent(*buffQue.front());
        }
        for (auto& it = managerListeners.first; it != managerListeners.second; ++it)
        {
            it->second->HandleEvent(*buffQue.front());
        }
        // will pop even if no one is listening
        BLEACH_DELETE(buffQue.front());
        buffQue.pop();
    }
}

#pragma once
#include <cstdint>

namespace nest
{
    using HashedId = uint64_t;
    class Event
    {
        HashedId m_eventId = 0;
        HashedId m_message = 0;
        bool m_disabled = false;
    public:
        Event(HashedId eventId = 0, HashedId message = 0)
            : m_eventId(eventId)
            , m_message(message)
        {}
        virtual HashedId GetId() const { return m_eventId; }
        virtual HashedId GetMessage() const { return m_message; }
        virtual void SetId(HashedId id) { m_eventId = id; }
        virtual void SetMessage(HashedId message) { m_message = message; }
        virtual void Disable() { m_disabled = true; }
        virtual bool GetDisabled() const { return m_disabled; }
        bool operator==(const Event& event) const
        {
            return m_eventId == event.m_eventId;
        }
    };

    struct OSTEventHash
    {
        HashedId operator()(const nest::Event* p) const
        {
            return p->GetId();
        }
    };
    struct OSTEventEqual
    {
        bool operator()(const nest::Event* lhs, const nest::Event* rhs) const
        {
            return lhs->GetId() == rhs->GetId();
        }
    };
};

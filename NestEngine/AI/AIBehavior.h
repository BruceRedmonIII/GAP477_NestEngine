#pragma once
#include "../HelperFiles/IdTypes.h"
namespace nest
{
    class AIBehavior
    {
        const HashedId m_id;
    public:
        enum class Status
        {
            kFinished,
            kActive,
            kFailed
        };
        AIBehavior(const HashedId id = 0) : m_id(id) {};
        virtual ~AIBehavior() = default;
        virtual Status Update([[maybe_unused]] float deltaTime) { return Status::kFailed; };
        virtual void Exit() {}
    };
}
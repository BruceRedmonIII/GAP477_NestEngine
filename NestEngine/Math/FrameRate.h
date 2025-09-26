#pragma once
#include <cstdint>
namespace nest
{
    struct FrameRate
    {
        uint32_t m_frameRate;
        float m_minDeltaTime;
        static inline int m_frameRateCounter = 0;
        FrameRate(uint32_t frameRate);
        void SetFrameRate(uint32_t frameRate);
        void CallDelay(float& deltaTime) const;
    };
}
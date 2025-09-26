#include "FrameRate.h"
#include <thread>

nest::FrameRate::FrameRate(uint32_t frameRate)
    : m_frameRate(frameRate)
{
    m_minDeltaTime = 1.f / static_cast<float>(m_frameRate);
}

void nest::FrameRate::SetFrameRate(uint32_t frameRate)
{
    m_frameRate = frameRate;
}
void nest::FrameRate::CallDelay(float& deltaTime) const
{
    float remainder = m_minDeltaTime - deltaTime;
    if (remainder > 0.f)
    {
        std::this_thread::sleep_for(std::chrono::duration<float>(remainder));
    }
}
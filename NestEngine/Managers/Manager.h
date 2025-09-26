#pragma once
#include "../Configuration/Macros.h"
namespace vk
{
    class Buffer;
    class CommandBuffer;
}
namespace GAP311
{
    struct ShaderUniform;
    struct RenderTarget;
}
namespace nest
{
    class Event;
    class Manager
    {
    public:
        SET_HASHED_ID(Manager)
        Manager() = default;
        virtual ~Manager() = default;
        virtual bool Init() { return false; }
        virtual void Exit() {};
        virtual void Update([[maybe_unused]] float deltaTime) {}
        virtual void ProcessInput() const {}
        virtual void PreRender() const {};
        virtual void HandleEvent([[maybe_unused]] const Event& evt) {}
        virtual void Render([[maybe_unused]] vk::CommandBuffer& commands) {}
        virtual void Destroy() {}
        virtual void UpdateVKBuffer([[maybe_unused]] vk::CommandBuffer& commands) {};
    };
}

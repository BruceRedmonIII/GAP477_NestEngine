#pragma once
namespace vk
{
    class Buffer;
    class Device;
}

namespace nest
{
    class RenderBufferImpl;
    class RenderBuffer
    {
        RenderBufferImpl* m_impl = nullptr;
    public:
        RenderBuffer();
        ~RenderBuffer();
        // Disable copy and move semantics
        RenderBuffer(const RenderBuffer&) = delete;
        RenderBuffer& operator=(const RenderBuffer&) = delete;
        RenderBuffer(RenderBuffer&&) = delete;
        RenderBuffer& operator=(RenderBuffer&&) = delete;
        vk::Buffer GetBuffer();
        void SetBuffer(vk::Buffer buffer);
    };
}
#pragma once
#include "RenderBuffer.h"
#include <vulkan/vulkan.hpp> // You can now use vk::Buffer
namespace nest
{
    class RenderBufferImpl
    {
        vk::Buffer m_buffer;
    public:
        vk::Buffer& GetBuffer() { return m_buffer; }
        void SetBuffer(const vk::Buffer& buffer) { m_buffer = buffer; }
        RenderBufferImpl() = default;
        ~RenderBufferImpl() = default;
    };
}
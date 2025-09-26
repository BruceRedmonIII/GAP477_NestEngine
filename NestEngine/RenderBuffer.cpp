#include "RenderBuffer.h"
#include "RenderBufferImpl.h"
#include "lib/BleachNew/BleachNew.h"

nest::RenderBuffer::RenderBuffer()
{
    m_impl = BLEACH_NEW(nest::RenderBufferImpl);
}
nest::RenderBuffer::~RenderBuffer()
{
    BLEACH_DELETE(m_impl);
}

vk::Buffer nest::RenderBuffer::GetBuffer()
{
    return m_impl->GetBuffer();;
}

void nest::RenderBuffer::SetBuffer(vk::Buffer buffer)
{
    m_impl->SetBuffer(buffer);
}

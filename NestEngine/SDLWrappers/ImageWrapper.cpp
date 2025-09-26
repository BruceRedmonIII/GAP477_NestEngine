#include "../Configuration/LogMacro.hint"
#include "ImageWrapper.h"
#include <SDL_render.h>
void nest::ImageWrapper::SetTexture(SDL_Texture* texture)
{
    if (texture != nullptr)
    {
        m_pTexture = texture;
        m_isValid = true;
    }
    else
        _LOG(LogType::kError, "Image received null texture!");
}

void nest::ImageWrapper::DestroyTexture()
{
    if (m_pTexture != nullptr)
    {
        SDL_DestroyTexture(m_pTexture);
        m_pTexture = nullptr;
    }
    m_isValid = false;
}
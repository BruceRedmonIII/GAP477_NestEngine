#pragma once
struct SDL_Texture;
namespace nest
{
    class ImageWrapper
    {
        bool m_isValid = false;
        SDL_Texture* m_pTexture = nullptr;
    public:
        ImageWrapper() = default;
        ~ImageWrapper() = default; // we purposely dont destroy the texture, as that gets destroyed in resource manager
        ImageWrapper(const ImageWrapper& copy) = delete;
        ImageWrapper& operator=(const ImageWrapper& copy) = delete;
        void SetTexture(SDL_Texture* texture);
        SDL_Texture* GetTexture() const { return m_pTexture; }
        bool IsValidTexture() const { return m_isValid; }
        void DestroyTexture();
    };
}

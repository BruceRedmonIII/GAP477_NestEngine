#pragma once
#include "Manager.h"
union SDL_Event;
namespace nest
{
    struct DescriptorSet;
    struct Texture;
    struct ImageView;
    struct Sampler;
    class ImGuiManager : public Manager
    {
        DescriptorSet* set;
        ImageView* view;
        Sampler* sampler;
    public:
        SET_HASHED_ID(ImGuiManager)
        bool Init() override;
        void ImGuiNewFrame() const;
        void PreRender() const override;
        void Render(vk::CommandBuffer& commands) override;
        void Destroy() override;
        void HandleEvent([[maybe_unused]] const Event& evt) override;
        void HandleSDLEvent(const SDL_Event& evt);
    private:
        bool ImGuiInitialize();
    };
}
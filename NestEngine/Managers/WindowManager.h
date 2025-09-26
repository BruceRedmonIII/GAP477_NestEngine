#pragma once
#include "Manager.h"
struct SDL_Window;
namespace nest
{
    class Window;
    class WindowManager : public Manager
    {
        Window* m_pWindow;
        const char* m_kTitle;
        int m_kWindowWidth;
        int m_kWindowHeight;
        bool m_isClosed;
    public:
        SET_HASHED_ID(WindowManager)
        WindowManager();
        bool Init() override;
        SDL_Window* GetWindow();
        bool Open(const char* title, int width, int height);
        bool Close();
        void Exit() override;
    };
}
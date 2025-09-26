#include "WindowManager.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../SDLWrappers/Window.h"
#include "../Configuration/LogMacro.hint"
#include <SDL.h>
nest::WindowManager::WindowManager()
    : m_pWindow(nullptr)
    , m_kTitle(nullptr)
    , m_kWindowWidth(0)
    , m_kWindowHeight(0)
    , m_isClosed(true)
{
}
bool nest::WindowManager::Init()
{
    m_pWindow = BLEACH_NEW(Window);
    return true;
}

bool nest::WindowManager::Open(const char* title, int width, int height)
{
    m_kTitle = title;
    m_kWindowWidth = width;
    m_kWindowHeight = height;
    if (!m_pWindow->Open(title, width, height)) // if window failed to create it will log itself, we just need to DestroyAll here
    {
        _LOG(LogType::kError, "Window failed to open!");
        return false;
    }
    m_isClosed = false;
    return true;
}

SDL_Window* nest::WindowManager::GetWindow()
{
    return m_pWindow->GetWindow();
}

// Attempt's to close window
// Returns true on close, or false if window is already closed
// NOTE: Does not delete window!
// Must call exit to deallocate, just like all managers
bool nest::WindowManager::Close()
{
    if (m_isClosed)
    {
        return false;
    }
    m_pWindow->Close();
    m_isClosed = true;
    return true;
}

void nest::WindowManager::Exit()
{
    Close();
    BLEACH_DELETE(m_pWindow);
}

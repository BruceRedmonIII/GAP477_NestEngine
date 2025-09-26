#include "Window.h"
#include "../Configuration/LogMacro.hint"
#include <filesystem>
#include <cassert>
#include <SDL.h>
nest::Window::Window()
    : m_pWindow(nullptr)
{}
SDL_Window* nest::Window::GetWindow()
{
    return m_pWindow;
}
//-----------------------------------------------------------------------------------------------
// Creates and opens a new window with the given params
//-----------------------------------------------------------------------------------------------
bool nest::Window::Open(const char* title, int width, int height)
{
    m_pWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN);
    if (!m_pWindow)
    {
        _LOG(LogType::kError, SDL_GetError());
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------------------------
// Closes the window
//-----------------------------------------------------------------------------------------------
void nest::Window::Close() const
{
    _LOG(LogType::kLog, "Closing window");
    SDL_DestroyWindow(m_pWindow);
}

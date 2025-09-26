#include "InputManager.h"
#include "EventManager.h"
#include "ImGuiManager.h"
#include "../Engine/Engine.h"
#include "../Events/EventIdHashes.h"
#include "../Configuration/Macros.h"
#include "../Events/Event.h"
#include "../Events/InputEvent.h"
#include "../SDLWrappers/Keycodes.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../Math/MathAlias.h"
#include <SDL.h>

nest::InputManager::InputManager()
{
#if _MANAGER_INIT_LOGGING == 1
    _LOG(LogType::kLog, "InputManager init successful!");
#endif
}
bool nest::InputManager::Init()
{
    SDL_StopTextInput(); // this will set the text input state to false as its not false by default (allows me to press q to close)
    return true;
}
//-----------------------------------------------------------------------------------------------
// Updates the current keys pressed
//-----------------------------------------------------------------------------------------------
void nest::InputManager::Update(float deltaTime)
{
    (void)deltaTime;
    SDL_Event sdlEvt;
    while (SDL_PollEvent(&sdlEvt))
    {
        nest::Engine::GetManager<nest::ImGuiManager>()->HandleSDLEvent(sdlEvt);
        if (sdlEvt.type == SDL_KEYDOWN) // when a key is pressed down or up, change the keyboard
        {
            m_board.HandleEvent(InputEvent{ static_cast<Keycodes>(sdlEvt.key.keysym.scancode), true});
        }
        else if (sdlEvt.type == SDL_KEYUP)
        {
            m_board.HandleEvent(InputEvent{ static_cast<Keycodes>(sdlEvt.key.keysym.scancode), false});
        }
        if (sdlEvt.type == SDL_MOUSEBUTTONDOWN)
        {
            if (sdlEvt.button.button == SDL_BUTTON_RIGHT)
            {
                m_board.HandleEvent(InputEvent{ Keycodes::Key_RightMouse, true });
            }
            else
            {
                m_board.HandleEvent(InputEvent{ Keycodes::Key_LeftMouse, true });
            }
        }
        else if (sdlEvt.type == SDL_MOUSEBUTTONUP)
        {
            if (sdlEvt.button.button == SDL_BUTTON_RIGHT)
            {
                m_board.HandleEvent(InputEvent{ Keycodes::Key_RightMouse, false });
            }
            else
            {
                m_board.HandleEvent(InputEvent{ Keycodes::Key_LeftMouse, false });
            }
        }
        if (sdlEvt.window.event == SDL_WINDOWEVENT_CLOSE) // when the windows X is clicked, set the OSTKey to true
        {
            m_board.SetKey(Keycodes::Key_WindowClose, true);
        }
        if (SDL_IsTextInputActive() == SDL_FALSE)
        {
            if (KeyPressed(Keycodes::Key_Q) ||
                KeyPressed(Keycodes::Key_WindowClose))
            {
                nest::Engine::GetInstance()->Quit(true);
            }
        }
    }
}

//-----------------------------------------------------------------------------------------------
// returns whether or not the key given is pressed
// runs in constant time
//-----------------------------------------------------------------------------------------------
bool nest::InputManager::KeyPressed(Keycodes key)
{
    return m_board.KeyPressed(key);
}

Vec2 nest::InputManager::GetMousePos()
{
    // need to give it a width and height for the overlap check
    int x, y = 0;
    SDL_GetMouseState(&x, &y);
    return {x, y};
}

#include "../Engine/Engine.h"
#include "../Managers/EventManager.h"
#include "KeyBind.h"
#include "../Events/InputEvent.h"
#include "Keyboard.h"
#include "Keycodes.h"
#include <SDL.h>
//-----------------------------------------------------------------------------------------------
// Keyboard constructor
//-----------------------------------------------------------------------------------------------
nest::Keyboard::Keyboard()
    : m_keyMap({})
{
    for (const auto key : m_kKeys)
    {
        m_keyMap.try_emplace(key, false);
    }
}

//-----------------------------------------------------------------------------------------------
// this will map the keys after a key is pressed to set it to true/false if relevant
//-----------------------------------------------------------------------------------------------
void nest::Keyboard::HandleEvent(const InputEvent& evt)
{
    if (evt.GetStatus() == true)
    {
        m_keyMap[evt.GetKey()] = true;
    }
    if (evt.GetStatus() == false)
    {
        m_keyMap[evt.GetKey()] = false;
    }
    nest::Engine::GetManager<EventManager>()->QueEvent(BLEACH_NEW(InputEvent(evt)));
}

//-----------------------------------------------------------------------------------------------
// Returns whether or not this key is pressed
// runs in constant time
//-----------------------------------------------------------------------------------------------
bool nest::Keyboard::KeyPressed(Keycodes key)
{
    return m_keyMap[key];
}

//-----------------------------------------------------------------------------------------------
// Checks if all keys that are part of this bind are pressed
//-----------------------------------------------------------------------------------------------
bool nest::Keyboard::KeyBindPressed(KeyBind* bind)
{
    for (int i = 0; i < bind->m_keyAmount; ++i)
    {
        if (m_keyMap[bind->m_pKeyBind[i]] == false)
            return false;
    }
    return true;
}
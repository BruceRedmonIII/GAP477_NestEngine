#pragma once
#include "Keycodes.h"
namespace nest
{
    // Set a scancode and check if it is pressed when asked
    class KeyBinder
    {
        Keycodes m_key = Keycodes::Key_Nothing;
    public:
        KeyBinder(Keycodes key)
            : m_key(key)
        {
        }
        KeyBinder() = default;
        bool IsPressed(Keycodes code) const
        {
            if (code == m_key)
                return true;
            return false;
        }
    };

    enum class Keycodes;
    struct KeyBind
    {
        Keycodes* m_pKeyBind;
        int m_keyAmount = 0;
        KeyBind(nest::Keycodes* keyBind)
            : m_pKeyBind(keyBind)
        {
        }
        static bool IsPressed() { return false; }
    };
}
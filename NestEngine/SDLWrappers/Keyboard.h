#pragma once
#include "Keycodes.h"
#include <unordered_map>
namespace nest
{
    class InputEvent;
    class Event;
    struct KeyBind;
    // OSTKeyboard maps all the keys on the keyboard to a bool variable
    // if a key is held down it is set to true, if it is released it is set to false
    class Keyboard
    {
        std::unordered_map<Keycodes, bool> m_keyMap;
    public:
        Keyboard();
        ~Keyboard() = default;
        Keyboard(const Keyboard& copy) = delete;
        Keyboard& operator=(const Keyboard& copy) = delete;
        void HandleEvent(const InputEvent& event);
        bool KeyPressed(Keycodes key);
        bool KeyBindPressed(KeyBind* bind);
        void SetKey(Keycodes key, bool status) { m_keyMap[key] = status; }
    };
}


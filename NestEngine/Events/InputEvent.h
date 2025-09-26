#pragma once
#include "EventIdHashes.h"
#include "Event.h"
#include "../SDLWrappers/Keycodes.h"

namespace nest
{
    class InputEvent :
        public Event
    {
        Keycodes m_keyboardScanCode = Keycodes::Key_Nothing;
        bool m_pressedDown;
        char m_uniKey = '\0';
    public:
        explicit InputEvent(Keycodes key, bool pressedDown = false, char uniKey = '\0')
            : Event(kInputEventId)
            , m_pressedDown(pressedDown)
            , m_uniKey(uniKey)
        {
            m_keyboardScanCode = key;
        }
        Keycodes GetKey() const { return m_keyboardScanCode; }
        char GetUniKey() const { return m_uniKey; }
        bool GetStatus() const { return m_pressedDown; }
    };
}

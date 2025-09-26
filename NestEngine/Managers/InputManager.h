#pragma once
#include "Manager.h"
#include "../Math/MathAlias.h"
#include "../Configuration/Macros.h"
#include "../SDLWrappers/Keyboard.h"
namespace nest
{
    enum class Keycodes;
    class Component;
    // InputManager stores a Keyboard class and uses that to check if a key is pressed
    // (or in the future, if a keybind is pressed)
    // I might not even need this class, but its here for now
    class InputManager : public Manager
    {
        Keyboard m_board;
    public:
        SET_HASHED_ID(InputManager)
        InputManager();
        ~InputManager() override = default;
        InputManager(const InputManager& copy) = delete;
        InputManager& operator=(const InputManager& copy) = delete;
        bool Init() override;
        // Update input state based on SDL events
        void Update(float deltaTime) override;
        bool KeyPressed(Keycodes key);
        Vec2 GetMousePos();
    };
}

#pragma once
#include "Component.h"
#include "../../DataStructs/kinematics.h"

namespace nest
{
    class GameObject;
    class TransformComponent3D;
    class KinematicComponent3D : public Component
    {
        GameObject* m_pOwner = nullptr;
        Kinematics m_kins;
        TransformComponent3D* m_transform = nullptr;
        bool m_rotateWithMouse = false; // used for cameras
        bool m_mousePressed = false; // keep track if player is holding right-mouse button
        float m_mouseX = 0; // set X when player clicks so we can track movement
        float m_mouseY = 0; // set Y when player clicks so we can track movement
        static constexpr float m_mouseSensitivity = .015f;
    public:
        SET_HASHED_ID(KinematicComponent3D)
        KinematicComponent3D() = default;
        ~KinematicComponent3D() override = default;
        bool Init(GameObject* owner) override;
        void Update(float deltaTime) override;
        void ProcessInput() override;
        void ClampRotationSpeed();
        void ClampLinearSpeed();
        void ResetAcceleration();
        void Forwards(bool status);
        void Backwards(bool status);
        void Left(bool status);
        void Right(bool status);
        void Up(bool status);
        void Down(bool status);
        void RotateWithMouse(bool status) { m_rotateWithMouse = status; }
        // delete later
        Kinematics& GetKins() { return m_kins; }
    private:
        void UpdateMove(float deltaTime);
        void ProcessMouseInput();
    };
}
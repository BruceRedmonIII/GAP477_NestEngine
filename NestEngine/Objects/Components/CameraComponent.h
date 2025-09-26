#pragma once
#include "Component.h"
#include "../../DataStructs/CameraData.h"
#include "../../DataStructs/kinematics.h"
#include "../../Uniforms/MeshDataStructs.h"
#include "../../Math/MathAlias.h"

namespace nest
{
    class GameObject;
    class TransformComponent3D;
    class CameraComponent : public Component
    {
        GameObject* m_owner = nullptr;
        Mat4 m_data;
        CameraConstants m_uniform;
        TransformComponent3D* m_transform = nullptr;
    public:
        SET_HASHED_ID(CameraComponent)
        CameraComponent() = default;
        ~CameraComponent() override = default;
        bool Init(GameObject* owner) override;
        void Update(float deltaTime) override;
        void HandleEvent(const Event& evt) override;
        Mat4 GetViewMatrix() const;
        void SetTransform(Mat4 data);
        CameraConstants& GetUniform() { return m_uniform; }
        void SetUniform(CameraConstants uniform) { m_uniform = uniform; }
    };
}
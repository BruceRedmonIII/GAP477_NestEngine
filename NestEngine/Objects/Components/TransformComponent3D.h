#pragma once
#include "Component.h"
#include "../GameObject.h"
#include "../../DataStructs/TransformData.h"
namespace nest
{
    // Forward Declares
    //----------------------------------------------------------------------------------------------------------//
    class GameObject;
    //----------------------------------------------------------------------------------------------------------//
    // TODO:: Tranform maybe shouldnt have size? maybe just position?
    class TransformComponent3D : public Component
    {
        GameObject* m_pOwner = nullptr;
        Mat4 m_transform = glm::identity<glm::mat4>();
        Vec3 m_position{};
        Vec3 m_right{};
        Vec3 m_up{};
        Vec3 m_forward{};
        float m_pitch = 0.f;
        float m_yaw = 0.f;
        float m_roll = 0.f;
    public:
        SET_HASHED_ID(TransformComponent3D)
        TransformComponent3D() = default;

        ~TransformComponent3D() override = default;
        TransformComponent3D(const TransformComponent3D& copy) = delete;
        TransformComponent3D& operator=(const TransformComponent3D& copy) = delete;
        TransformComponent3D(const TransformComponent3D&& copy) = delete;
        TransformComponent3D& operator=(const TransformComponent3D&& copy) = delete;

        bool Init(GameObject* owner) override;
        Mat4 GetTransform() const { return m_transform; }
        Mat4 GetRotationMatrix() const;
        Vec3 GetPosition() const { return m_position; }
        Vec3& GetPositionRef() { return m_position; }
        Mat4& GetMatrixRef() { return m_transform; }
        Mat4 LookAt() const;
        void SetTransform(const Mat4& data);
        void SetPosition(const Vec3& position);
        void AddPosition(const Vec3& position);
        void SetRotation(const Mat4& rotation);
        void AddRotation(float yaw, float pitch);
        float& Yaw() { return m_yaw; }
        float& Pitch() { return m_pitch; }
        float& Roll() { return m_roll; }
        void SetScale(const Vec3& scale);
    };
}

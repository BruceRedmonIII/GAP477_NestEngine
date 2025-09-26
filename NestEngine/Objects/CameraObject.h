#pragma once
#include "GameObject.h"
namespace nest
{
    class KinematicComponent3D;
    class CameraComponent;
    struct CameraConstants;
    class CameraObject : public GameObject
    {
        CameraComponent* m_camera = nullptr;
        KinematicComponent3D* m_kins = nullptr;
    public:
        CameraObject() = default;
        ~CameraObject() override;
        bool Init() override;
        CameraConstants& GetCameraUniform();
        void SetCameraUniform(const CameraConstants& uniform);
        void SetTransform(const Mat4& transform);
        void Forwards(bool status);
        void Backwards(bool status);
        void Left(bool status);
        void Right(bool status);
        void Up(bool status);
        void Down(bool status);
    };
}
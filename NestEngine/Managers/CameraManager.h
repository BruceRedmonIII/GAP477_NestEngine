#pragma once
#include "Manager.h"
#include "../Math/MathAlias.h"

namespace nest
{
	struct CameraConstants;
    class CameraObject;
	class Event;
    // Holds the active camera and provides access to its uniform data
	class CameraManager : public Manager
	{
		nest::CameraObject* m_pActiveCamera = nullptr;
	public:
		static GAP311::ShaderUniform s_cameraUniform;
		static vk::Buffer s_cameraConstantsBuffer;
	public:
        SET_HASHED_ID(CameraManager)
		CameraManager() = default;
		~CameraManager() override = default;
		bool Init() override;
		void Exit() override; 
		void Update(float deltaTime) override;
		CameraConstants GetActiveCameraUniform() const;
		CameraConstants& GetActiveCameraUniformRef();
		void SetActiveCamera(nest::CameraObject* camera);
		void SetCameraTransform(const Mat4& mat);
		void SetCameraUniform(CameraConstants uniform);
		CameraObject* GetCamera() { return m_pActiveCamera; }
		void ProcessInput() const;
		void UpdateVKBuffer(vk::CommandBuffer& commands) override;
	};
}
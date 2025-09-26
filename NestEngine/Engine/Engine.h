#pragma once
#include "../Managers/Manager.h"
#include "../DataWrappers/HashMapWrapper.h"

template <class ManagerType> concept IsManager = std::is_base_of_v<nest::Manager, ManagerType>;
namespace nest
{
	using Managers = HashMapWrapper<Manager>;
	class ResourceManager;
	class GameObjectManager;
	class InputManager;
	class EventManager;
	class Window;
	class GraphicsSystem;
	class LogManager;
    class Scene;
	class UIManager;
	// Engine is used to initialize everything including singletons, and deallocate them all when closed.
	// contains the window in which we display the game and also the log system where we store our logs.
	// (this probably doesn't need the log system here)
	class Engine
	{
		static Engine* s_pEngine;
		uint32_t m_frameRate;
		static Managers* m_pManagers;
		LogManager* m_pLogSystem;
		GraphicsSystem* m_pGraphics;
		bool m_quit;
		Engine();
	public:
		~Engine();
		Engine(const Engine& copy) = delete;
		Engine& operator=(const Engine& copy) = delete;
		static Engine* GetInstance();
		static GraphicsSystem* GetGraphics();
		static bool CreateInstance(const char* title, int width, int height, uint32_t frameRate);
		void GameLoop();
		void DestroyAllSystems() const;

		template <IsManager ManagerType>
		static ManagerType* GetManager() { return m_pManagers->GetContained<ManagerType>(); }

		template <IsManager ManagerType>
		static ManagerType* GetManager(uint64_t id) { return m_pManagers->GetContainedById<ManagerType>(id); }

		template <IsManager ManagerType>
		static bool AddManager(ManagerType* manager) { return m_pManagers->Emplace<ManagerType>(manager); };

		static void RenderManagers(vk::CommandBuffer& commands);

		void Quit(bool status) { m_quit = status; }
		uint32_t GetFrameRate() { return m_frameRate; }

		void UpdateManagerVKBuffers(vk::CommandBuffer& commands);
	private:
		bool InitAllSystems(const char* title, int width, int height, uint32_t frameRate);
		bool InitSDL();
		// Loads all default file path locations to FileManger
		void SetDefaultFilePaths();
	};
}


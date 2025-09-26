#include "Engine.h"
#include "../Objects/Components/Component.h"
#include "../Managers/EventManager.h"
#include "../Managers/FactoryManager.h"
#include "../Managers/GraphicsSystem.h"
#include "../DataWrappers/HashMapWrapper.h"
#include "../Managers/InputManager.h"
#include "../Managers/LogManager.h"
#include "../Managers/ImGuiManager.h"
#include "../Managers/CameraManager.h"
#include "../Managers/LightManager.h"
#include "../Configuration/Macros.h"
#include "../Managers/ResourceManager.h"
#include "../Managers/WindowManager.h"
#include "../Managers/SceneManager.h"
#include "../Managers/FileManager.h"
#include "../GlobalConstants/UIConstants.h"
#include "../SDLWrappers/Window.h"
#include "../Scene/Scene.h"
#include "../Configuration/EngineConfig.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../Math/Random.h"
#include "../Math/FrameRate.h"
#include "../VulkanWrappers/Vulkan.h"
#include "../gap-311/VulkanWrapper.hpp"
#include "../Managers/SkyboxManager.h"
#include "../GlobalConstants/constants.h"
#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <ranges>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#ifdef _SDL_IMAGE
#include <SDL_image.h>
#endif

nest::Engine* nest::Engine::s_pEngine = nullptr;
nest::Managers* nest::Engine::m_pManagers = BLEACH_NEW(nest::Managers());
//-----------------------------------------------------------------------------------------------
// EngineConstructor
//-----------------------------------------------------------------------------------------------
nest::Engine::Engine()
    : m_frameRate(60)
    , m_pLogSystem(nullptr)
    , m_quit(false)
{
}

//-----------------------------------------------------------------------------------------------
// EngineDestructor, will dump the leak detector
//-----------------------------------------------------------------------------------------------
nest::Engine::~Engine()
{
    BLEACH_DUMP_AND_DESTROY_LEAK_DETECTOR();
}

void nest::Engine::RenderManagers(vk::CommandBuffer& commands)
{
    auto& managers = m_pManagers->GetAllComponents();
    for (auto& manager : std::views::values(managers))
    {
        manager->Render(commands);
    }
}

void nest::Engine::UpdateManagerVKBuffers(vk::CommandBuffer& commands)
{
    auto& managers = m_pManagers->GetAllComponents();
    for (auto& manager : std::views::values(managers))
    {
        manager->UpdateVKBuffer(commands);
    }
}

//-----------------------------------------------------------------------------------------------
// Init's all applicable systems including SDL
//-----------------------------------------------------------------------------------------------
bool nest::Engine::InitAllSystems(const char* title, int width, int height, uint32_t frameRate)
{
    m_frameRate = frameRate;

    //////////////////////////////////////////////////////////////////////
    /// INIT LOG
    //////////////////////////////////////////////////////////////////////
    m_pLogSystem = BLEACH_NEW(LogManager()); // LogSystem inits itself
    FileManager::GetInstance();
    SetDefaultFilePaths();
    InitSDL();

    WindowManager* pWindowManager = BLEACH_NEW(WindowManager);
    pWindowManager->Init();
    if (pWindowManager->Open(title, width, height)) // if window failed to create it will log itself, we just need to DestroyAll here
    {
        AddManager(pWindowManager);
    }

    GraphicsSystem* pGraphicsSystem = BLEACH_NEW(GraphicsSystem);
    if (!pGraphicsSystem->Init())
    {
        _LOG_V(nest::LogType::kError, "Graphics failed to Init!");
        DestroyAllSystems();
        return false;
    }
    m_pGraphics = pGraphicsSystem;
    AddManager(BLEACH_NEW(InputManager()));
    AddManager(BLEACH_NEW(EventManager()));
    AddManager(BLEACH_NEW(SceneManager()));
    ResourceManager* m_pResourceManager = BLEACH_NEW(ResourceManager());
    if (m_pResourceManager->Init())
    {
        AddManager(m_pResourceManager);
    }

    FactoryManager* pFactoryManager = BLEACH_NEW(FactoryManager);
    if (pFactoryManager->Init())
    {
        AddManager(pFactoryManager);
    }

    CameraManager* pCameraManager = BLEACH_NEW(nest::CameraManager);
    if (pCameraManager->Init())
    {
        AddManager(pCameraManager);
    }

    LightManager* pLightManager = BLEACH_NEW(nest::LightManager);
    if (pLightManager->Init())
    {
        AddManager(pLightManager);
    }
    ImGuiManager* pImGuiManager = BLEACH_NEW(nest::ImGuiManager);
    if (pImGuiManager->Init())
    {
        AddManager(pImGuiManager);
    }
    const char* imagePaths[] =
    {
        "miramar_ft.png", // Front
        "miramar_bk.png", // Back
        "miramar_up.png", // Up
        "miramar_dn.png", // Down
        "miramar_rt.png", // Right
        "miramar_lf.png", // Left
    };
    SkyboxManager* pSkyboxManager = BLEACH_NEW(nest::SkyboxManager);
    if (pSkyboxManager->Init(imagePaths))
    {
        AddManager(pSkyboxManager);
    }
    return true;
}

bool nest::Engine::InitSDL()
{
    //////////////////////////////////////////////////////////////////////
        /// INIT SDL
        //////////////////////////////////////////////////////////////////////
        // returns a negative error code if it failed to initialize
    int errorCode = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    if (errorCode < 0)
    {
        _LOG_V(LogType::kError, errorCode, ' ', SDL_GetError()); // if SDL_init returned an error, this will give us more information
        DestroyAllSystems();
        return false;
    }
#if _ENGINE_BASIC_LOGGING == 1
    _LOG(nest::LogType::kLog, "SDL_Init() was successful");
#endif

    //////////////////////////////////////////////////////////////////////
    /// INIT SDL_IMAGE
    //////////////////////////////////////////////////////////////////////

#ifdef _SDL_IMAGE
    int imgFlags = IMG_INIT_PNG;
    // returns the flags that were successfully initialized
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags)
    {
        _LOG_V(LogType::kError, "SDL_image initialization failed: ", SDL_GetError());
        DestroyAllSystems();
        return false;
    }
#endif
    //////////////////////////////////////////////////////////////////////
    /// INIT SDL_TTF
    //////////////////////////////////////////////////////////////////////
    errorCode = TTF_Init();
    // returns a negative error code if it failed to initialize
    if (errorCode < 0)
    {
        _LOG_V(LogType::kError, "TTFL_Init failed. Error code ", errorCode, ": ", SDL_GetError());
        DestroyAllSystems();
        return false;
    }
#if _ENGINE_BASIC_LOGGING == 1
    _LOG(nest::LogType::kLog, "TTF_Init() succeeded.");
#endif

    //////////////////////////////////////////////////////////////////////
    /// INIT SDL_MIXER
    //////////////////////////////////////////////////////////////////////
    // returns a negative error code if it failed to initialize
    errorCode = Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT,
        MIX_DEFAULT_CHANNELS, 1024);
    if (errorCode < 0)
    {
        _LOG_V(LogType::kError, "Mix_OpenAudio() failed. Error: ", Mix_GetError());
        DestroyAllSystems();
        return false;
    }
#if _ENGINE_BASIC_LOGGING == 1
    _LOG(nest::LogType::kLog, "Mix_OpenAudio() succeeded.");
#endif
    return true;
}

void nest::Engine::SetDefaultFilePaths()
{
    for (const auto& value : s_kAssetPaths)
        nest::FileManager::AddPaths(s_kAssetKey, value);
    for (const auto& value : s_kXMLPaths)
        nest::FileManager::AddPaths(s_kXMLFilesKey, value);
    for (const auto& value : s_kSkyboxPaths)
        nest::FileManager::AddPaths(s_kSkyboxKey, value);
}

//-----------------------------------------------------------------------------------------------
// Updates/Draws/ProcessInputs/SendsMessages for all relevant systems one time
//-----------------------------------------------------------------------------------------------
void nest::Engine::GameLoop()
{
    // Get the current time, high-res clock is the fastest
    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    FrameRate frameRate{ m_frameRate };
    InputManager* m_manager = GetManager<InputManager>();
    while (!m_quit)
    {
        // Get another instance of high-res clocks current time for comparison
        auto thisFrameTime = std::chrono::high_resolution_clock::now();

        // Get the duration that has passed.
        std::chrono::duration<float> lastFrameDuration = thisFrameTime - lastFrameTime;

        // Convert duration into a double for delta time.
        float deltaTime = static_cast<float>(lastFrameDuration.count());

        auto& managers = m_pManagers->GetAllComponents();
        for (auto& manager : std::views::values(managers))
        {
            manager->ProcessInput();
            manager->Update(deltaTime);
            manager->PreRender();
        }
        m_pGraphics->Render();

        frameRate.CallDelay(deltaTime);

        // Changes the frame, without this there's just 1 long frame
        lastFrameTime = thisFrameTime;
        if (m_manager->KeyPressed(Keycodes::Key_WindowClose) || m_manager->KeyPressed(Keycodes::Key_Q))
            m_quit = true;

#if _SHOW_FRAMERATE == 1
        frameRate.PrintFrameRate(deltaTime);
#endif

#if _SHOW_DELTA_TIME == 1
        frameRate.PrintDeltaTime(deltaTime);
#endif
    }
}

//-----------------------------------------------------------------------------------------------
// Destroys all systems relevant
//-----------------------------------------------------------------------------------------------
void nest::Engine::DestroyAllSystems() const
{
    // this is in reverse order from the init list
#if _ENGINE_BASIC_LOGGING == 1
    _LOG(nest::LogType::kLog, "Destroying all systems!");
#endif
    GetGraphics()->m_vulkanWrapper->vulkan.GetDevice().waitIdle();
    auto& managers = m_pManagers->GetAllComponents();
    for (auto& manager : std::views::values(managers))
    {
        manager->Exit();
        manager->Destroy();
    }
    BLEACH_DELETE(m_pManagers);
    m_pGraphics->Exit();
    BLEACH_DELETE(m_pGraphics);
    nest::FileManager::Destroy();
    m_pLogSystem->CloseLog();
    BLEACH_DELETE(m_pLogSystem);
    BLEACH_DELETE(s_pEngine);
}

//-----------------------------------------------------------------------------------------------
// Attempt to gather an instance of the engine, if none are found, create a new instance
// Init systems and return true if successful, false otherwise.
//-----------------------------------------------------------------------------------------------
bool nest::Engine::CreateInstance(const char* title, int width, int height, uint32_t frameRate)
{
    // don't make 2 engines
    assert(!s_pEngine);
    s_pEngine = BLEACH_NEW(Engine());
    return s_pEngine->InitAllSystems(title, width, height, frameRate);
}

nest::Engine* nest::Engine::GetInstance()
{
    assert(s_pEngine);
    return s_pEngine;
}

nest::GraphicsSystem* nest::Engine::GetGraphics()
{
    assert(s_pEngine);
    return s_pEngine->m_pGraphics;
}

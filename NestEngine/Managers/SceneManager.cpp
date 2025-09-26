#include "WindowManager.h"
#include "../lib/BleachNew/BleachNew.h"
#include "../SDLWrappers/Window.h"
#include "../Configuration/LogMacro.hint"
#include <SDL.h>
#include "SceneManager.h"
#include <cassert>

// Attemps to close window
// Returns true on close, or false if window is already closed
// NOTE: Does not delete window!
// Must call exit to deallocate, just like all managers

bool nest::SceneManager::Init()
{
    m_pScenes.reserve(1); // Expect at-least 1 scene
    return true;
}

// Checks if Scene has already been added, if so returns false
// Returns true on successful Add
// DOES NOT make added scene the active scene
bool nest::SceneManager::AddScene(Scene* newScene)
{
    Scene* scene = GetScene(newScene->GetId());
    if (scene != nullptr) // dont allow adding a scene if the id's are the same
        return false;

    m_pScenes.emplace_back(newScene);
    return true;
}

bool nest::SceneManager::RemoveScene(HashedId id)
{
    Scene* scene = GetScene(id);
    if (scene == nullptr)
        return false;

    BLEACH_DELETE(scene);
    scene = nullptr;
    m_pScenes.shrink_to_fit();
    return true;
}

bool nest::SceneManager::SetActiveScene(HashedId id)
{
    Scene* scene = GetScene(id);
    if (scene == nullptr) // if scene isnt found, dont make it the active scene
        return false;
    m_pActiveScene = scene;
    return true;
}

nest::Scene* nest::SceneManager::GetScene(HashedId id)
{
    for (Scene* scene : m_pScenes)
    {
        if (scene->GetId() == id)
        {
            return scene;
        }
    }
    return nullptr;
}

nest::Scene* nest::SceneManager::GetActiveScene()
{
    if (m_pActiveScene != nullptr)
        return m_pActiveScene;
    return nullptr;
}

void nest::SceneManager::PreRender() const
{
    assert(m_pActiveScene);
    m_pActiveScene->PreRender();
}

void nest::SceneManager::Exit()
{
    for (Scene* scene : m_pScenes)
    {
        scene->Exit();
        BLEACH_DELETE(scene);
    }
}
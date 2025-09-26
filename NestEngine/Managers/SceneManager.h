#pragma once
#include "Manager.h"
#include "../Scene/Scene.h"
#include <vector>
namespace nest
{
    class SceneManager : public Manager
    {
        std::vector<Scene*> m_pScenes;
        Scene* m_pActiveScene;
    public:
        SET_HASHED_ID(SceneManager)
        bool Init() override;
        bool AddScene(Scene* newScene);
        bool RemoveScene(HashedId id);
        bool SetActiveScene(HashedId id);
        Scene* GetScene(HashedId id);
        Scene* GetActiveScene();
        void PreRender() const override;
        void Exit() override;
    };
}
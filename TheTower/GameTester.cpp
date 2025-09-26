#include "GameTester.h"
#include "../NestEngine/Math/Hash.h"
#include "../NestEngine/Math/FrameRate.h"
#include "../NestEngine/Factories/GameFactory.h"
#include "../NestEngine/Managers/GraphicsSystem.h"
#include "../NestEngine/Managers/LogManager.h"
#include "../NestEngine/Parsing/XMLFile.h"
#include "../NestEngine/Scene/Scene.h"
#include "../NestEngine/lib/BleachNew/BleachNew.h"
#include "../NestEngine/Engine/Engine.h"
#include "../NestEngine/Nodes/Node.h"
#include "../NestEngine/Managers/SceneManager.h"
#include "../NestEngine/Managers/FileManager.h"
#include "config.h"
#include <chrono>
#include <ctime>

GameTester::GameTester()
	: m_pCurrentScene(nullptr)
    , m_pEngine(nullptr)
	, m_quit(false)
{}

bool GameTester::Init()
{
	m_pEngine = nest::Engine::GetInstance();
	m_pCurrentScene = BLEACH_NEW(nest::Scene);
	m_pCurrentScene->Init(nest::Hash("Test"));
	m_pEngine->GetManager<nest::SceneManager>()->AddScene(m_pCurrentScene);
	m_pEngine->GetManager<nest::SceneManager>()->SetActiveScene(nest::Hash("Test"));
	nest::FileManager::AddPaths(s_kAssetKey, "Assets/PBR/", "NestEngine/Assets/PBR/", "../NestEngine/Assets/PBR/");
	LoadSceneXML(nest::FileManager::ResolvePath(s_kXMLFilesKey, "Loader.xml").c_str());
	m_pEngine->GetGraphics()->InitPipelines(m_pCurrentScene);
	return true;
}

void GameTester::GameLoop()
{
	m_pEngine->GameLoop();
}

bool GameTester::Exit()
{
	m_pEngine->DestroyAllSystems();
	nest::LogManager::CloseLog();
	return true;
}

void GameTester::LoadSceneXML(const char* location)
{
	// this should be the loader file
	nest::XMLFile file{ location };
	nest::GameFactory factory;

	// parse loader file to get LoadFile attribute which has the real file location
	const char* attribute{};; // I'm giving this a base
	const char* name{}; // I'm not demanding a name as its not really used yet
	// print file name if debug mode
	if (file.GetConstCharFromRoot("Name", name))
	{
		_LOG_V(nest::LogType::kLog, "Loading file - ", name);
	}
	else
	{
		name = "DefaultName";
	}

	if (file.GetConstCharFromRoot("LoadFile", attribute))
	{
		nest::XMLFile mainFile{ attribute };

		// load real file
		if (mainFile.Check())
		{
			if (factory.LoadScene(*m_pCurrentScene, mainFile) == nest::Result::kSucess)
			{
				return; // return early to avoid error message
			}
		}
	}
	_LOG_V(nest::LogType::kError, "Loading file failed! - Name = ", name, "| Address = ", attribute);
}
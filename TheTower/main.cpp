#include "../NestEngine/Scene/Scene.h"
#include "../NestEngine/DataStructs/TransformData.h"
#include "../NestEngine/lib/BleachNew/BleachNew.h"
#include "../NestEngine/Engine/Engine.h"
#include "config.h"
#include "GameTester.h"
int main()
{
	BLEACH_INIT_LEAK_DETECTOR();
	if (!nest::Engine::CreateInstance("Tower", g_windowWidth, g_windowHeight, g_windowFPS))
	{
		_LOG(nest::LogType::kError, "Engine failed to init!");
		return 0;
	}
    GameTester* tester = BLEACH_NEW(GameTester);
	tester->Init(); 
	tester->GameLoop();
    tester->Exit();
    BLEACH_DELETE(tester);
	BLEACH_DUMP_AND_DESTROY_LEAK_DETECTOR();
	return 0;
}
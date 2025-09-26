#include "FileManager.h"
#include "../lib/BleachNew/BleachNew.h"
nest::FileManager* nest::FileManager::s_pFileManager = nullptr;

nest::FileManager* nest::FileManager::GetInstance()
{
	if (s_pFileManager == nullptr)
		s_pFileManager = BLEACH_NEW(FileManager);
	return s_pFileManager;
}

void nest::FileManager::Destroy()
{
	BLEACH_DELETE(s_pFileManager);
}
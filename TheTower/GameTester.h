#pragma 
namespace nest
{
	class Scene;
	class GraphicsSystem;
    class Engine;
}
class GameTester
{
    nest::Engine* m_pEngine;
	nest::Scene* m_pCurrentScene;
	bool m_quit;
public:
	GameTester();
	bool Init();
	void GameLoop();
	bool Exit();
private:
	void LoadSceneXML(const char* location);
};

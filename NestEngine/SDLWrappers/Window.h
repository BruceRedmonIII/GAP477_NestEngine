#pragma once
struct SDL_Window;
namespace nest
{ 
    class Window
    {
    private:
        SDL_Window* m_pWindow = nullptr;
    public:
        Window();

        SDL_Window* GetWindow();
        bool Open(const char* title, int width, int height);
        void Close() const;
    };
}
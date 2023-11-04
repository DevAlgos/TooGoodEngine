#pragma once

class Application
{
public:
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    Application();
    ~Application();

    void MainLoop();

    static Graphics::Window& GetMainWindow();

    long long GetCurrentTime();
    static float GetCurrentDelta();

private:
    Utils::LayerManager Manager;
    

};
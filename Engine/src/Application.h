#pragma once

class Application
{
public:
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    Application();
    ~Application();

    void Run();

    static TGE::Window& GetMainWindow();

    long long GetCurrentTime();

    static float GetCurrentDelta(); //In miliseconds
    static float GetCurrentDeltaSecond(); //In seconds

private:
    Utils::LayerManager Manager;
    

};
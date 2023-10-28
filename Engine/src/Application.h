#pragma once

class Application
{
public:
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    Application();
    ~Application();

    void MainLoop();
    void MainLoop(MemoryData stats);

    static Graphics::Window& GetMainWindow();


private:
    static void DisplayDebugInfo(MemoryData stats);


};
#pragma once

#include <Window.h>
#include <Utils/Layers/LayerManager.h>
#include <UserApplication.h>

namespace TooGoodEngine 
{

    class Application
    {
    public:
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        Application(const TooGoodEngine::UserApplication& App);
        ~Application();

        void Run();

        static TooGoodEngine::Window& GetMainWindow();

        int64_t GetCurrentTime();

        static float GetCurrentDelta(); //In miliseconds
        static float GetCurrentDeltaSecond(); //In seconds

    private:
        Utils::LayerManager Manager;


    };
}
#pragma once

#include <Window.h>
#include <Utils/Layers/LayerManager.h>
#include <UserApplication.h>

namespace TGE 
{

    class Application
    {
    public:
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        Application(const TGE::UserApplication& App);
        ~Application();

        void Run();

        static TGE::Window& GetMainWindow();

        long long GetCurrentTime();

        static float GetCurrentDelta(); //In miliseconds
        static float GetCurrentDeltaSecond(); //In seconds

    private:
        Utils::LayerManager Manager;


    };
}
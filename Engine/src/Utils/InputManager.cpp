#include "pch.h"
#include "InputManager.h"

namespace 
{
    static GLFWwindow* s_Window = nullptr;
    static double s_xOffset = 0.0f;
    static double s_yOffset = 0.0f;
    static bool s_Scrolled = false;

    static void ScrollCallBack(GLFWwindow* window, double xOffset, double yOffset)
    {
        s_Scrolled = true;
        s_xOffset = xOffset;
        s_yOffset = yOffset;

    }
}

namespace InputManager 
{
    void BeginPolling(GLFWwindow* window)
    {
        assert(!(s_Window != nullptr) && "Input Manager Already Initialized!");

        if (!s_Window)
            s_Window = window;

        glfwSetScrollCallback(s_Window, ScrollCallBack);

    }

    const bool IsKeyReleased(int key)
    {
        auto state = glfwGetKey(s_Window, key);
        return state == GLFW_RELEASE;
    }

    const bool IsKeyPressed(int key)
    {
        auto state = glfwGetKey(s_Window, key);
        return state == GLFW_PRESS;
    }

    const bool IsKeyDown(int key)
    {
        auto state = glfwGetKey(s_Window, key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    const bool IsMouseButtonPressed(int button)
    {
        auto state = glfwGetMouseButton(s_Window, button);
        return state == GLFW_PRESS;
    }

    const bool IsMouseButtonReleased(int button)
    {
        auto state = glfwGetMouseButton(s_Window, button);
        return state == GLFW_RELEASE;
    }

    const bool IsMouseButtonDown(int button)
    {
        auto state = glfwGetMouseButton(s_Window, button);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    const bool IsScrolled()
    {
        return s_Scrolled;
    }

    void SetScrolledToFalse()
    {
        s_Scrolled = false;
    }

    const std::pair<double, double> GetScrollWheel()
    {
        return std::pair<double, double>(s_xOffset, s_yOffset);
    }


    void GetMousePos(double& x, double& y)
    {
        glfwGetCursorPos(s_Window, &x, &y);
    }


}
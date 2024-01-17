#include "pch.h"
#include "Input.h"

namespace 
{
    static GLFWwindow* s_Window = nullptr;
    static double s_xOffset = 0.0f;
    static double s_yOffset = 0.0f;
    static bool s_Scrolled = false;

    static double s_LastX = 0.0;
    static double s_LastY = 0.0;

    static bool s_CursorEnabled = true;

    static void ScrollCallBack(GLFWwindow* window, double xOffset, double yOffset)
    {
        s_Scrolled = true;
        s_xOffset = xOffset;
        s_yOffset = yOffset;

    }
}

namespace Input 
{
    void BeginPolling(GLFWwindow* window)
    {
        assert(!(s_Window != nullptr) && "Input Already Initialized!");

        s_Window = window;

        glfwSetScrollCallback(s_Window, ScrollCallBack);
    }

    const bool IsKeyReleased(int key)
    {
        int state = glfwGetKey(s_Window, key);
        return state == GLFW_RELEASE;
    }

    const bool IsKeyPressed(int key)
    {
        int state = glfwGetKey(s_Window, key);
        return state == GLFW_PRESS;
    }

    const bool IsKeyDown(int key)
    {
        int state = glfwGetKey(s_Window, key);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    const bool IsMouseButtonPressed(int button)
    {
        auto state = glfwGetMouseButton(s_Window, button);
        return state == GLFW_PRESS;
    }

    const bool IsMouseButtonReleased(int button)
    {
        int state = glfwGetMouseButton(s_Window, button);
        return state == GLFW_RELEASE;
    }

    const bool IsMouseButtonDown(int button)
    {
        int state = glfwGetMouseButton(s_Window, button);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    const bool IsScrolled()
    {
        return s_Scrolled;
    }

    const bool IsCursorEnabled()
    {
        return s_CursorEnabled;
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

    void DisableCursor()
    {
       glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
       s_CursorEnabled = false;
    }

    void EnableCursor()
    {
        glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        s_CursorEnabled = true;
    }


}
#include <Pulsar/Private/InputInterface.h>
#include <ThirdParty/glfw/include/GLFW/glfw3.h>
//#include <Windows.h>

namespace pulsar::detail
{
    bool InputInterface::GetKey(const int& keyCode)
    {
        //return glfwGetKey(g_glfw_window_instance, (int)keyCode) == GLFW_PRESS;
        return false;
    }

    bool InputInterface::GetMouseButton(const int& keyCode)
    {
        //return glfwGetMouseButton(g_glfw_window_instance, keyCode);
        return false;
    }

    void InputInterface::GetCursorPosition(int* out_x, int* out_y)
    {
        double x, y;
        //glfwGetCursorPos(g_glfw_window_instance, &x, &y);
        *out_x = (int)x;
        *out_y = (int)y;
        //POINT p;
        //GetCursorPos(&p);
        //*out_x = p.x;
        //*out_y = p.y;
    }

    void InputInterface::PollEvents()
    {
        glfwPollEvents();
    }


}


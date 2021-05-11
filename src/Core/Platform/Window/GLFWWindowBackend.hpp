#ifndef CORE_PLATFORM_WINDOW_GLFWWINDOWBACKEND_HPP
#define CORE_PLATFORM_WINDOW_GLFWWINDOWBACKEND_HPP

#include "Core/Platform/Window/IWindowBackend.hpp"

#include <GLFW/glfw3.h>

namespace Core::Platform::Window {

class GLFWWindowBackend : public IWindowBackend {
public:
    GLFWWindowBackend(std::string_view title, const Dimensions &size);
    ~GLFWWindowBackend() override;

    // Inherited Functions
    void create() override;
    void update() override;
    bool exited() override;

    // Only available if GLFWWindowBackend is instantiated directly.
    void setWindowHint(int hint, int value);
    GLFWwindow *getWindow();

private:
    GLFWwindow *windowHandle{nullptr}; /** @brief A reference to the window backend*/

};

}



#endif //CORE_PLATFORM_WINDOW_GLFWWINDOWBACKEND_HPP

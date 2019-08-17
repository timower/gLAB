#include "gLAB/window.h"

#include <glad/glad.h>

#include "gLAB/utils.h"

void errorCb(int code, const char* error) {
    fprintf(stderr, "GLFW error: %d\n %s \n", code, error);
}

GLFWwindow* createWindow(int width, int height) {
    glfwSetErrorCallback(errorCb);
    ASSERT(glfwInit(), "GLFW init failed!");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window =
        glfwCreateWindow(width, height, "Hello, bgfx!", nullptr, nullptr);

    ASSERT(window, "GLFW window is null");

    glfwMakeContextCurrent(window);

    // disable V-sync:
    glfwSwapInterval(0);

    return window;
}

void initGL(GLFWwindow* window) {
    ASSERT(gladLoadGL(), "Unable to initialize openGL");

    // Make a dummy GL call (we don't actually need the result)
    // IF YOU GET A CRASH HERE: it probably means that you haven't initialized
    // the OpenGL function loader used by this code. Desktop OpenGL 3/4 need a
    // function loader. See the IMGUI_IMPL_OPENGL_LOADER_xxx explanation above.
    GLint current_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);
}

void setGlfwCallbacks(GLFWwindow* window, const GLFWcallbacks& callbacks) {
    if (callbacks.windowSizeCb != nullptr)
        glfwSetWindowSizeCallback(window, callbacks.windowSizeCb);

    if (callbacks.cursorPosCb != nullptr)
        glfwSetCursorPosCallback(window, callbacks.cursorPosCb);

    if (callbacks.charCb != nullptr)
        glfwSetCharCallback(window, callbacks.charCb);

    if (callbacks.mouseButtonCb != nullptr)
        glfwSetMouseButtonCallback(window, callbacks.mouseButtonCb);

    if (callbacks.keyCb != nullptr) glfwSetKeyCallback(window, callbacks.keyCb);

    if (callbacks.scrollCb != nullptr)
        glfwSetScrollCallback(window, callbacks.scrollCb);

    if (callbacks.cursorEnterCb != nullptr)
        glfwSetCursorEnterCallback(window, callbacks.cursorEnterCb);
}

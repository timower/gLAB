#include "gLAB/window.h"

#include "gLAB/utils.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#define ENTRY_CONFIG_USE_WAYLAND 0

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#if ENTRY_CONFIG_USE_WAYLAND
#include <wayland-egl.h>
#define GLFW_EXPOSE_NATIVE_WAYLAND
#else
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#elif BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif  //
#include <GLFW/glfw3native.h>

void errorCb(int code, const char *error) {
    fprintf(stderr, "GLFW error: %d\n %s \n", code, error);
}

GLFWwindow *createWindow(int width, int height) {

	glfwSetErrorCallback(errorCb);
    ASSERT(glfwInit(), "GLFW init failed!");

    // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window =
        glfwCreateWindow(width, height, "Hello, bgfx!", nullptr, nullptr);

    ASSERT(window, "GLFW window is null");

    return window;
}

void initBgfx(GLFWwindow *window, int width, int height) {
    bgfx::PlatformData platformData;

    if (width < 0 || height < 0) {
        glfwGetWindowSize(window, &width, &height);
    }

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    platformData.nwh =
        reinterpret_cast<void *>(uintptr_t(glfwGetGLXWindow(window)));
    platformData.ndt = glfwGetX11Display();
    platformData.context = glfwGetGLXContext(window);
#elif BX_PLATFORM_OSX
    platformData.nwh = glfwGetCocoaWindow(window);
#elif BX_PLATFORM_WINDOWS
    platformData.nwh = glfwGetWin32Window(window);
#endif  // BX_PLATFORM_

    bgfx::setPlatformData(platformData);

    bgfx::Init bgfxInit;
    bgfxInit.type =
        bgfx::RendererType::Count;  // Automatically choose a renderer.
    bgfxInit.resolution.width = uint32_t(width);
    bgfxInit.resolution.height = uint32_t(height);
    bgfxInit.resolution.reset = BGFX_RESET_NONE;  // BGFX_RESET_VSYNC;

    ASSERT(bgfx::init(bgfxInit), "Can't init bgfx!");

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x0, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
}

void setGlfwCallbacks(GLFWwindow *window, const GLFWcallbacks &callbacks) {
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

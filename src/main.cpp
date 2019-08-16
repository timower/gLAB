#include <cstdio>
#include <exception>

#include <GLFW/glfw3.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <bx/file.h>
#include <bx/os.h>

#include "gLAB/application.h"
#include "gLAB/gui.h"
#include "gLAB/utils.h"
#include "gLAB/window.h"

#include <dlfcn.h>

// TODO: windows / mac support for .dll / ...
constexpr auto DefaultAppPath = "libapp.so";
constexpr auto CreateAppFuncName = "createApplication";
using CreateAppFunc = Application *(*)(void);

int main(int /*argc*/, char * /*argv*/[]) {
    bx::FileInfo fileInfo;
    auto appPath = DefaultAppPath;
    if (!bx::stat(fileInfo, appPath)) {
        die("Can't find application library");
    }

    auto appLib = dlopen("./libapp.so", RTLD_LOCAL | RTLD_LAZY);
    if (!appLib) {
        fprintf(stderr, "%s\n", dlerror());
        die("Error loading application library");
    }
    CreateAppFunc appFunc =
        reinterpret_cast<CreateAppFunc>(bx::dlsym(appLib, CreateAppFuncName));

    if (!appFunc) {
        die("Application library doesn't contain create function");
    }
    auto app = appFunc();

    auto *window = createWindow(800, 600);
    initBgfx(window);

    setGlfwCallbacks(window, getImguiCallbacks(nullptr));

    bgfx::setDebug(BGFX_DEBUG_TEXT);

    imguiCreate();

    app->init();

    unsigned int counter = 0;
    auto lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        const auto time = glfwGetTime();
        const auto dt = time - lastTime;
        lastTime = time;

        bgfx::touch(0);

        imguiBeginFrame();

        app->drawUI();

        imguiEndFrame();

        app->update(dt);

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx::frame();

        counter++;
    }

    imguiDestroy();

    glfwDestroyWindow(window);

    return 0;
}

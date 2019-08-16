#include <cstdio>
#include <exception>

#define DLL_EXPORT

#include <GLFW/glfw3.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <bx/file.h>
#include <bx/os.h>

#include "gLAB/application.h"
#include "gLAB/gui.h"
#include "gLAB/utils.h"
#include "gLAB/window.h"

#include "app.h"

int main(int /*argc*/, char * /*argv*/[]) {
    auto *window = createWindow(800, 600);
    initBgfx(window);

    setGlfwCallbacks(window, getImguiCallbacks(nullptr));

    bgfx::setDebug(BGFX_DEBUG_TEXT);

    imguiCreate();

    auto& app = getExampleApp(window);

    app.init();

    unsigned int counter = 0;
    auto lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        const auto time = glfwGetTime();
        const auto dt = time - lastTime;
        lastTime = time;

        bgfx::touch(0);

        imguiBeginFrame();

        app.drawUI();

        imguiEndFrame();

        app.update(dt);

        // Advance to next frame. Rendering thread will be kicked to
        // process submitted rendering primitives.
        bgfx::frame();

        counter++;
    }

    imguiDestroy();

    glfwDestroyWindow(window);

    return 0;
}

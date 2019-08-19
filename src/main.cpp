#include <cstdio>
#include <exception>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "gLAB/application.h"
#include "gLAB/gui.h"
#include "gLAB/utils.h"
#include "gLAB/window.h"

#include "app.h"

#include <stdio.h> /* defines FILENAME_MAX */

#include <direct.h>
#define GetCurrentDir _getcwd

int main(int /*argc*/, char* /*argv*/[]) {
    char cCurrentPath[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
        return errno;
    }

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

    printf("The current working directory is %s", cCurrentPath);

    auto* window = createWindow(800, 600);
    initGL(window);

    setGlfwCallbacks(window, getImguiCallbacks(nullptr));

    imguiCreate(window);

    auto& app = getExampleApp(window);

    app.init();

    unsigned int counter = 0;
    auto lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto time = glfwGetTime();
        const auto dt = time - lastTime;
        lastTime = time;

        imguiBeginFrame();

        app.drawUI();

        imguiEndFrame();

        app.update(dt);

        counter++;
        glfwSwapBuffers(window);
    }

    imguiDestroy();

    glfwDestroyWindow(window);

    return 0;
}

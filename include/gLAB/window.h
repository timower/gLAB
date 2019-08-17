#pragma once

#include "../lib/glabexternal.h"

struct GLFWcallbacks {
    /// windowSizeCb(window, width, height)
    void (*windowSizeCb)(GLFWwindow *, int, int);

    /// cursorPosCb(window, x, y)
    void (*cursorPosCb)(GLFWwindow *, double, double);

    /// charCb(widnow, codepoint)
    void (*charCb)(GLFWwindow *, unsigned int);

    /// mouseButtonCb(window, button, actions, mods)
    void (*mouseButtonCb)(GLFWwindow *, int, int, int);

    /// keyCb(window, key, sconcode, action, mods)
    void (*keyCb)(GLFWwindow *, int, int, int, int);

    /// scrollCb(window, dx, dy)
    void (*scrollCb)(GLFWwindow *, double, double);

    /// cursorEnterCb(window, mode)
    void (*cursorEnterCb)(GLFWwindow *, int);
};

GLFWwindow *createWindow(int width, int height);

void initGL(GLFWwindow *window);

void setGlfwCallbacks(GLFWwindow *window, const GLFWcallbacks &callbacks);

#pragma once

// TODO: fix this include?
#include "../lib/glabexternal.h"

class Application {
   public:
    Application(GLFWwindow* window) {
        this->window = window;
    }

    virtual ~Application() {
    }

    virtual void init() = 0;
    virtual void drawUI() = 0;
    virtual void update(double dt) = 0;

   protected:
    GLFWwindow* window;
};

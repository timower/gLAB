//#include "../lib/glabexternal.h"
#include "gLAB/all.h"

class ExampleApplication : public Application {
   private:
    float test;
    char sourceBuffer[1024];

    void readSource() {
    }

    void writeSource() {
    }

   public:
    ExampleApplication(GLFWwindow* w) : Application(w) {
    }

    void init() override {
        test = 0;
    }

    void drawUI() override {
        if (ImGui::Begin("Source Editor")) {
            ImGui::InputTextMultiline("##", sourceBuffer, sizeof(sourceBuffer));
        }
        ImGui::End();
    }

    void update(double dt) override {
    }
};

extern "C" Application* createApplication(GLFWwindow* window) {
    return new ExampleApplication(window);
}

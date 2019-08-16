//#include "../lib/glabexternal.h"
#include "app.h"
#include "gLAB/all.h"


class ExampleApplication : public Application {
   private:
    float test = 0;
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

Application& getExampleApp(GLFWwindow* w) {
    static ExampleApplication ex(w);
    return ex;
}

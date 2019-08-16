//#include "../lib/glabexternal.h"
#include "app.h"
#include "gLAB/all.h"


class ExampleApplication : public Application {
   private:
    float avgFps = 0;
    float Fps = 0;
    float passedTime = 0;
    char buf[512];

    void readSource() {
    }

    void writeSource() {
    }

   public:
    ExampleApplication(GLFWwindow* w) : Application(w) {
    }

    void init() override {}

    void drawUI() override {
    
		ImGui::Begin("Window Start");
        ImGui::Text("Hello, world %d", 123);
        if (ImGui::Button("Save")) {
            // do stuff
        }
        ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
        ImGui::Text("FPS %.0f", avgFps);
        ImGui::End();
	}

    void update(double dt) override {
        passedTime += dt;
        Fps += 1;
        if (passedTime > 1) {
            avgFps = Fps / passedTime;
            Fps = 0;
            passedTime = 0;
        }
    }
};

Application& getExampleApp(GLFWwindow* w) {
    static ExampleApplication ex(w);
    return ex;
}

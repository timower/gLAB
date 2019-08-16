//#include "../lib/glabexternal.h"
#include "app.h"
#include "gLAB/all.h"


class ExampleApplication : public Application {
   private:
    float f = 0;
    char buf[512];

    void readSource() {
    }

    void writeSource() {
    }

   public:
    ExampleApplication(GLFWwindow* w) : Application(w) {
    }

    void init() override {
        f = 0.0001;
    }

    void drawUI() override {
    
		ImGui::Begin("Window Start");
        ImGui::Text("Hello, world %d", 123);
        if (ImGui::Button("Save")) {
            // do stuff
        }
        ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::End();
	}

    void update(double dt) override {
        f = 1/dt;
    }
};

Application& getExampleApp(GLFWwindow* w) {
    static ExampleApplication ex(w);
    return ex;
}

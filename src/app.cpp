//#include "../lib/glabexternal.h"
#include "app.h"
#include "gLAB/all.h"

#include "gLAB/math/transform.h"
#include "gLAB/shader.h"

#include <array>

std::array<float, 3 * 3> vertices = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                                     0.0f,  0.0f,  0.5f, 0.0f};

template <typename Bindalbe>
struct BindGuard {
    BindGuard(const Bindalbe& b) : _ref(b) {
        b.bind();
    }

    ~BindGuard() {
        _ref.unbind();
    }

   private:
    const Bindalbe& _ref;
};

template <auto Target = GL_ARRAY_BUFFER>
struct GraphicsBuffer {
    static constexpr GLenum Target = Target;

    GLuint GLbuffer;

    GraphicsBuffer() {
        glGenBuffers(1, &GLbuffer);
    }

    ~GraphicsBuffer() {
        glDeleteBuffers(1, &GLbuffer);
    }

    GraphicsBuffer(const GraphicsBuffer&) = delete;
    GraphicsBuffer& operator=(const GraphicsBuffer&) = delete;

    void fill(void* data, std::size_t size, GLenum usage = GL_STATIC_DRAW) {
        BindGuard<GraphicsBuffer> _(*this);
        glBufferData(Target, size, data, usage);
    }

    void bind() const {
        glBindBuffer(Target, GLbuffer);
    }

    void unbind() const {
        glBindBuffer(Target, 0);
    }

    // TODO: fill parts using glSubBufferData
};

struct GraphicsBufferLayout {
    GLuint VAO;

    GraphicsBufferLayout() {
        glGenVertexArrays(1, &VAO);
    }

    ~GraphicsBufferLayout() {
        glDeleteVertexArrays(1, &VAO);
    }

    GraphicsBufferLayout(const GraphicsBufferLayout&) = delete;
    GraphicsBufferLayout& operator=(const GraphicsBufferLayout&) = delete;

    GraphicsBufferLayout& begin() {
        glBindVertexArray(VAO);
        return *this;
    }

    GraphicsBufferLayout& add(const GraphicsBuffer<>& buffer, int location,
                              int size, GLenum type, unsigned stride = 0,
                              unsigned offset = 0) {
        BindGuard<GraphicsBuffer<>> _(buffer);
        glVertexAttribPointer(location, size, type, GL_FALSE, stride,
                              (void*)offset);
        glEnableVertexAttribArray(location);

        return *this;
    }

    void end() {
        glBindVertexArray(0);
    }
};

struct GraphicsObject {
    Shader shader;
    GraphicsBufferLayout layout;

    void draw(GLuint start, GLuint end) {
        shader.activate();
        layout.begin();
        glDrawArrays(GL_TRIANGLES, start, end);
        layout.end();
    }
};

class ExampleApplication : public Application {
   private:
    float avgFps = 0;
    float frames = 0;
    float passedTime = 0;

    GraphicsBuffer<> VBO;

    GraphicsObject triangle;

   public:
    ExampleApplication(GLFWwindow* w) : Application(w) {
    }

    // TODO: remove init and just use constructor?
    void init() override {
        VBO.fill(&vertices[0], sizeof(float) * vertices.size());

        triangle.shader.load("shaders/test.vs", "shaders/test.fs");
        triangle.layout.begin().add(VBO, 0, 3, GL_FLOAT).end();
    }

    void drawUI() override {
        ImGui::Begin("Window Start");
        ImGui::Text("FPS %.0f", avgFps);
        ImGui::End();
    }

    void update(double dt) override {
        passedTime += dt;
        frames += 1;
        if (passedTime > 1) {
            avgFps = frames / passedTime;
            frames = 0;
            passedTime = 0;
        }

        triangle.draw(0, 3);
    }
};

Application& getExampleApp(GLFWwindow* w) {
    static ExampleApplication ex(w);
    return ex;
}

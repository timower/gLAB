#pragma once

#include <glad/glad.h>

#include <string_view>

// TODO: move assignment and constructors
class Shader {
   private:
    GLuint program = 0;

   public:
    Shader();
    Shader(std::string_view vertexPath, std::string_view fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void load(std::string_view vertexPath, std::string_view fragmentPath);

    void activate();
};
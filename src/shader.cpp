#include "gLAB/shader.h"

#include "gLAB/utils.h"

#include <fstream>
#include <iostream>

static GLuint loadShader(std::string_view path, GLuint shaderType) {
    std::string source;
    std::ifstream t(&path[0]);

    ASSERT(!t.fail(), "Error opening shader file");

    t.seekg(0, std::ios::end);
    source.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    source.assign((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());

    GLuint shader = glCreateShader(shaderType);
    const char* src_ptr = &source[0];
    glShaderSource(shader, 1, &src_ptr, nullptr);

    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
        die("");
    }

    return shader;
}

Shader::Shader() {
}

Shader::Shader(std::string_view vertexPath, std::string_view fragmentPath) {
    load(vertexPath, fragmentPath);
}

Shader::~Shader() {
    if (program) {
        glDeleteProgram(program);
    }
}

void Shader::load(std::string_view vertexPath, std::string_view fragmentPath) {
    ASSERT(!vertexPath.empty(), "Empty shader path");
    ASSERT(!fragmentPath.empty(), "Empty shader path");
    auto vertexShader = loadShader(vertexPath, GL_VERTEX_SHADER);
    auto fragmentShader = loadShader(fragmentPath, GL_FRAGMENT_SHADER);
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
        die("");
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::activate() {
    ASSERT(program != 0, "Activating uninitialized shader");
    glUseProgram(program);
}

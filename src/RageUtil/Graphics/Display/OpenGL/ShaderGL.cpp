#include "ShaderGL.hpp"

#include "GLHelper.hpp"
#include "RageUtil/File/RageFile.h"

#include <fmt/format.h>
#include <GL/glew.h>

#include <vector>
#include <stdexcept>

/**
 * A shader constructor incase there is only a single shader type.
 * Convenience ctor to help with refactoring RageDisplay_OGL
 * @param type Shader type
 * @param path Shader path relative to app directory
 */
ShaderGL::ShaderGL(unsigned int type, const ghc::filesystem::path &path) : programID(0) {
    if(type == GL_VERTEX_SHADER) this->addVertexShader(path);
    if(type == GL_FRAGMENT_SHADER) this->addFragmentShader(path);
    this->compile();
}

ShaderGL::ShaderGL(const ghc::filesystem::path &vertexPath, const ghc::filesystem::path &fragmentPath) : programID(0) {
    this->addVertexShader(vertexPath);
    this->addFragmentShader(fragmentPath);
    this->compile();
}

ShaderGL::~ShaderGL() {
    GLCall(glDeleteProgram(this->programID));
}

void ShaderGL::addVertexShader(const ghc::filesystem::path &vertexPath) {
    unsigned vertexID = ShaderGL::compileShader(GL_VERTEX_SHADER, vertexPath);
    this->shaderIDs.push_back(vertexID);
}

void ShaderGL::addFragmentShader(const ghc::filesystem::path &fragmentPath) {
    unsigned fragmentID = ShaderGL::compileShader(GL_FRAGMENT_SHADER, fragmentPath);
    this->shaderIDs.push_back(fragmentID);
}

void ShaderGL::compile() {
    GLCall(this->programID = glCreateProgram());
    for(auto const& id : this->shaderIDs){
        GLCall(glAttachShader(programID, id));
    }
     GLCall(glLinkProgram(programID));
}

void ShaderGL::bind() const {
    GLCall(glUseProgram(this->programID));
}

void ShaderGL::unbind() {
    GLCall(glUseProgram(0));
}

void ShaderGL::setUniform1i(const std::string &name, int value) {
    GLCall(glUniform1i(this->getUniformLocation(name), value));
}

int ShaderGL::getUniformLocation(const std::string &name) const {
    GLCall(int location = glGetUniformLocation(programID, name.c_str()));
    if(location == -1)
        std::cout << "Warning: Uniform \"" << name << "\" does not exist.\n";
    return location;
}

unsigned int ShaderGL::compileShader(unsigned int type, const ghc::filesystem::path &path) {
    // Load Source
    RageFile file;
    if (!file.Open(path))
        throw std::runtime_error(fmt::format("Error compiling shader {}: {}", path.string(), file.GetError()));
    std::string source;
    file.Read(source, file.GetFileSize());

    // Create and compile shader
    auto shaderID = glCreateShader(type);
    auto src = reinterpret_cast<const GLchar *>(source.c_str());
    GLCall(glShaderSource(shaderID, 1, &src, nullptr));
    GLCall(glCompileShader(shaderID));

    // Validate
    int success;
    GLCall(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success));
    if(success == GL_TRUE)
        return shaderID;

    // Only reached if shader compilation fails.
    // Get length of error message
    GLint maxLength = 0;
    GLCall(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength));

    // Get error message and put in errorLog
    std::vector<GLchar> errorLog(maxLength);
    GLCall(glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]));

    GLCall(glDeleteShader(shaderID)); // Don't leak the shader.
    throw std::runtime_error(fmt::format("Shader compilation failed: {}", errorLog.data()));
}

bool ShaderGL::isActive() {
    return this->programID != 0;
}

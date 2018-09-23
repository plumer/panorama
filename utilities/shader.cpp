#include "shader.h"
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <fstream>
//#include "../glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

// utility
int readFile(const char * fileName, char * & content);

SubShader::SubShader()
    : _shaderType(GL_INVALID_ENUM) {
    _fileName = nullptr;
    _shaderObj = 0;
} 

void
SubShader::generate(GLenum type, const char * filename) {
    _shaderType = type;
    _fileName = filename;
    if (!isValid()) {
        _shaderObj = glCreateShader(_shaderType);
        if (_shaderObj == 0) {
            const char * typeStr = (type == GL_VERTEX_SHADER ? "vertex" :
                    (type == GL_GEOMETRY_SHADER ? "geometry" :
                     (type == GL_FRAGMENT_SHADER ? "fragment" : "unknown")));
            fprintf(stderr, "Error creating %s shader\n", typeStr);
            exit(1);
        }
    }
	FILE * test;
    fopen_s(&test, _fileName, "r");
    assert(test != nullptr);
	fclose(test);
}

SubShader::~SubShader() {
    if (this -> isValid()) {
        glDeleteShader(_shaderObj);
    }
}

void
SubShader::compile() {
    char * text;
    readFile(_fileName, text);
    // compile general shader
    const GLchar *p[1];
    GLint lengths[1];

    p[0] = text;
    lengths[0] = strlen(text);
    glShaderSource(_shaderObj, 1, p, lengths);
    glCompileShader(_shaderObj);

    GLint success;
    glGetShaderiv(_shaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(_shaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader %s: '%s'\n", _fileName, InfoLog);
        exit(1);
    }
    delete [] text;
}


Shader::Shader() {
    _createProgram();
}

Shader::Shader(const char * vs, const char * fs, const char * gs) {
    _createProgram();
    if (vs) _vertexShader.generate(GL_VERTEX_SHADER, vs);
    if (fs) _fragmentShader.generate(GL_FRAGMENT_SHADER, fs);
    if (gs) _geometryShader.generate(GL_GEOMETRY_SHADER, gs);
}

void
Shader::useNone() {
    glUseProgram(0);
}

void
Shader::_createProgram() {
    _program = glCreateProgram();

    if (_program == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }
}

void
Shader::feedVertexShader(const char * fileName) {
    _vertexShader.generate(GL_VERTEX_SHADER, fileName);
}

void
Shader::feedFragmentShader(const char * fileName) {
    _fragmentShader.generate(GL_VERTEX_SHADER, fileName);
}

void
Shader::feedGeometryShader(const char * fileName) {
    _geometryShader.generate(GL_VERTEX_SHADER, fileName);
}

Shader &
Shader::compile() {
    if (_vertexShader.isValid()) {
        _vertexShader.compile();
        glAttachShader(_program, _vertexShader.shaderObj());
    }
    if (_fragmentShader.isValid()) {
        _fragmentShader.compile();
        glAttachShader(_program, _fragmentShader.shaderObj());
    }
    if (_geometryShader.isValid()) {
        _geometryShader.compile();
        glAttachShader(_program, _geometryShader.shaderObj());
    }

    return *this;
}

Shader &
Shader::link() {
    GLint success = 0;
    GLchar errorLog[1024] = {0};
    glLinkProgram(_program);
    glGetProgramiv(_program, GL_LINK_STATUS, &success);
    if (success == 0) {
        glGetProgramInfoLog(_program, sizeof(errorLog), NULL, errorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", errorLog);
        exit(1);
    }

    // Sensitive code on Mac OS below.
#ifndef __APPLE__
    glValidateProgram(_program);
    glGetProgramiv(_program, GL_VALIDATE_STATUS, &success);
    if (success == 0) {
        glGetProgramInfoLog(_program, sizeof(errorLog), NULL, errorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", errorLog);
        exit(1);
    }
#endif
    return *this;
}

void
Shader::use() const {
    glUseProgram(_program);
}

GLuint
Shader::getUniformLocation(const char * variableName) const {
    GLuint location = glGetUniformLocation(_program, variableName);
    if (location == 0xFFFFFFFF) {
        fprintf(stderr, "location not found for variable '%s'\n", variableName);
        assert(0);
    }
    return location;
}

void
Shader::uniformMat4(const char * variableName, const GLfloat * value) const {
    GLuint location = getUniformLocation(variableName);
    glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void Shader::uniformMat4(const char *variableName, const glm::mat4 &m) const
{
	GLuint location = getUniformLocation(variableName);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::uniformMat3(const char *varName, const GLfloat *value) const {
	GLuint location = getUniformLocation(varName);
	glUniformMatrix3fv(location, 1, GL_FALSE, value);
}

void
Shader::uniform1f(const char * variableName, GLfloat value) const {
    GLuint location = getUniformLocation(variableName);
    glUniform1f(location, value);
}

void
Shader::uniform1i(const char * variableName, GLint value) const {
    GLuint location = getUniformLocation(variableName);
    glUniform1i(location, value);
}

void Shader::uniform2fv(const char *variableName, const GLfloat *value) const
{
	GLuint location = getUniformLocation(variableName);
	glUniform2fv(location, 1, value);
}

void Shader::uniform2fv(const char * variableName, const glm::vec2 &v) const
{
	GLuint location = getUniformLocation(variableName);
	glUniform2fv(location, 1, glm::value_ptr(v));
}

void
Shader::uniform3fv(const char * variableName, const GLfloat * value) const {
    GLuint location = getUniformLocation(variableName);
    glUniform3fv(location, 1, value);
}

void
Shader::uniform3fv(const char * variableName, const glm::vec3 &v) const {
	GLuint location = getUniformLocation(variableName);
	glUniform3fv(location, 1, glm::value_ptr(v));
}

int readFile(const char *filename, char * & content) {
	FILE * file;
	fopen_s(&file, filename, "rb");
    if (!file) {
        fprintf(stderr, "file '%s' does not exist\n", filename);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    content = new char [length+1];
    rewind(file);
    fread(content, length, sizeof(char), file);
    content[length] = 0;
    //printf("file name '%s', content:\n%s\n", filename, content);
    return 1;
}

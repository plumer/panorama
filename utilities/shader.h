#ifndef __MIMIC_SHADER_H__
#define __MIMIC_SHADER_H__

#include "glad/glad.h"
#include <glm/glm.hpp>

class SubShader {
    private:
        const char * _fileName;
        GLuint _shaderObj;
        GLenum _shaderType;
    public:
        SubShader();
        ~SubShader();

        void generate(GLenum type, const char * filename = nullptr);
        void compile();

        GLuint shaderObj() const {
            return _shaderObj;
        }
        bool isValid() const {
            return glIsShader(_shaderObj);
        }
};

class Shader {
    GLuint _program;
    SubShader _vertexShader;
    SubShader _fragmentShader;
    SubShader _geometryShader;

    void _createProgram();

    public:
        Shader();
        Shader(const char * vs, const char * fs, const char *gs = nullptr);
        Shader(const Shader &) = delete;
        Shader & operator = (const Shader &) = delete;

        static void useNone();

        Shader & compile();
        Shader & link();
        void use() const;

        void feedVertexShader(const char *);
        void feedFragmentShader(const char *);
        void feedGeometryShader(const char *);
        //void compileVertexShader();
        //void compileFragmentShader();
        //void compileGeometryShader();

        GLuint getUniformLocation(const char *) const;
        void uniform1f  (const char *, GLfloat) const;
        void uniform1i  (const char *, GLint)   const;
		void uniform2fv (const char *, const GLfloat *) const;
		void uniform2fv (const char *, const glm::vec2 &) const;
        void uniform3fv (const char *, const GLfloat *) const;
		void uniform3fv (const char *, const glm::vec3 &) const;
        void uniformMat4(const char *, const GLfloat *) const;
		void uniformMat4(const char *, const glm::mat4 &) const;
		void uniformMat3(const char *, const GLfloat *) const;

    private:
        void _compileGeneralShader(GLuint, const char *);
        //GLuint _createShader(GLenum);

};

#endif


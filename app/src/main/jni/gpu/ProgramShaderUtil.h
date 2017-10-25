//
// Created on 2017/10/25.
//
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#ifndef AALIVE_PROGRAMSHADERUTIL_H
#define AALIVE_PROGRAMSHADERUTIL_H
class ProgramShaderUtil{
public:
    static GLuint loadShader(GLenum type, const GLchar* string);
    static GLuint createProgram(const GLchar* vertexSource, const GLchar* fragmentSource);
};
#endif //AALIVE_PROGRAMSHADERUTIL_H

//
// Created on 2017/10/25.
//

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#ifndef AALIVE_SHADER_H
#define AALIVE_SHADER_H

class Shader{
public:
    Shader();
    ~Shader();
    void create(const GLchar* vertexSource, const GLchar* fragmentSource);
    void release();
    void useProgram();
    GLint getAttribLocation(const GLchar* name);
    GLint getUniformLocation(const GLchar* name);
private:
    GLuint mProgram;
};
#endif //AALIVE_SHADER_H

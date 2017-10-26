//
// Created on 2017/10/25.
//
#include "Shader.h"
#include "ProgramShaderUtil.h"
#include "GLUtil.h"

Shader::Shader():mProgram(0){
}

Shader::~Shader(){
    release();
}

void Shader::create(const GLchar* vertexSource, const GLchar* fragmentSource){
    mProgram = ProgramShaderUtil::createProgram(vertexSource, fragmentSource);
    GLUtil::checkGLError("Shader::create createProgram");
}

void Shader::release(){
    if(0 != mProgram){
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
}

void Shader::useProgram(){
    glUseProgram(mProgram);
}

GLint Shader::getAttribLocation(const GLchar* name){
    GLint location = glGetAttribLocation(mProgram, name);
    return location;
}

GLint Shader::getUniformLocation(const GLchar* name){
    GLint location = glGetUniformLocation(mProgram, name);
    return location;
}

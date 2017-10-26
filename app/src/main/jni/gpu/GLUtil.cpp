//
// Created on 2017/10/26.
//
#include "GLUtil.h"
#include "Logger.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

void GLUtil::checkGLError(const std::string& msg){
    checkGLError(msg.c_str());
}

void GLUtil::checkGLError(const char* msg){
    GLenum error = glGetError();
    if(GL_NO_ERROR != error){
        if(msg){
            LOGE("%s, GLerr: %d", msg, error);
        }else{
            LOGE("GLerr: %d", error);
        }
    }
}
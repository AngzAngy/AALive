#include "Framebuffer.h"

Framebuffer::Framebuffer():mFramebufferId(0){
}

Framebuffer::~Framebuffer(){
    release();
}

void Framebuffer::create(){
    glGenFramebuffers(1, &mFramebufferId);
}

void Framebuffer::release(){
    if(0 != mFramebufferId){
        glDeleteFramebuffers(1, &mFramebufferId);
        mFramebufferId = 0;
    }
}

void Framebuffer::bindTexture(GLenum textureTarget,GLenum texture,GLuint textureId){
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);

    glActiveTexture(texture);
    glBindTexture(textureTarget, textureId);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureTarget, textureId, 0);
}

void Framebuffer::unbindTexture(GLenum textureTarget, GLenum texture){
    glActiveTexture(texture);
    glBindTexture(textureTarget, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



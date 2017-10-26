#include "Framebuffer.h"

Framebuffer::Framebuffer():mFramebufferId(0){
}

void Framebuffer::create(){
    release();
}

void Framebuffer::release(){
    if(0 != mFramebufferId){
        glDeleteFramebuffers(1, &mFramebufferId);
        mFramebufferId = 0;
    }
}

void Framebuffer::bindTexture(GLenum textureTarget, GLuint textureId){
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureTarget, textureId, 0);
}

void Framebuffer::unbind(GLenum textureTarget){
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureTarget, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer(){
    glDeleteFramebuffers(1, &mFramebufferId);
};



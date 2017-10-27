#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifndef AALIVE_FRAMEBUFFER_H
#define AALIVE_FRAMEBUFFER_H

class Framebuffer {
public:

    Framebuffer();
    ~Framebuffer();
    void create();
    void release();
    void bindTexture(GLenum textureTarget,GLenum texture,GLuint textureId);
    void unbindTexture(GLenum textureTarget,GLenum texture);
    GLuint getFramebufferId(){
        return mFramebufferId;
    }

private:
    GLuint mFramebufferId;

};
#endif

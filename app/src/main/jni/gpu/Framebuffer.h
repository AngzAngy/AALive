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
    void bindTexture(GLenum textureTarget,GLuint textureId);
    void unbind(GLenum textureTarget);
    GLuint getFramebufferId(){
        return mFramebufferId;
    }

private:
    GLuint mFramebufferId;

};
#endif

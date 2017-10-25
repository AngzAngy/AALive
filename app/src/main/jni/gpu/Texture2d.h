#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifndef _Included_TEXTURE2D_H
#define _Included_TEXTURE2D_H


class Texture2d {

public:
    Texture2d();

    ~Texture2d();

    void genTexture(GLsizei w, GLsizei h, GLint format, GLenum type = GL_UNSIGNED_BYTE, GLvoid* pixels=NULL);
    void subImage(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
    GLsizei getWidth();
    GLsizei getHeight();
    GLint getColorFormart();

    GLuint getTextureId();


private:
    GLuint mTextureId;
    GLsizei mWidth;
    GLsizei mHeight;
    GLint mColorFormat; // only support RGBA now

};

#endif

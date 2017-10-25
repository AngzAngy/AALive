#include "Texture2d.h"

/*static int toolsavefile(unsigned char* pData, int size, char* name)
{
    FILE *pf =fopen(name, "wb");
    if(pf != 0){
        fwrite(pData, size, 1, pf);
        fclose(pf);
        return 1;

       }
    return 0;
}

static int toolloadfile(unsigned char* pData, int size, char* name)
{

    FILE *pf =fopen(name, "rb");
    if(pf != 0){
        fread(pData, size, 1, pf);
        fclose(pf);
        return 1;

       }
    return 0;
}*/

Texture2d::Texture2d():mWidth(0),mHeight(0),mColorFormat(0),mTextureId(0){

}

Texture2d::~Texture2d(){
    glDeleteTextures(1, &mTextureId);
}

void Texture2d::genTexture(GLsizei w, GLsizei h, GLint format, GLenum type, GLvoid* pixels){
    mWidth = w;
    mHeight = h;
    mColorFormat = format;

    glGenTextures(1, &mTextureId);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // This is necessary for non-power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, type, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2d::subImage(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels){
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glTexSubImage2D (GL_TEXTURE_2D, 0, xoffset,  yoffset, width, height, format, type, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLsizei Texture2d::getWidth(){
    return mWidth;
}

GLsizei Texture2d::getHeight(){
    return mHeight;
}

GLint Texture2d::getColorFormart(){
    return mColorFormat;
}

GLuint Texture2d::getTextureId(){
    return mTextureId;
}

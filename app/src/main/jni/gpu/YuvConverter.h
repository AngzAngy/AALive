//
// Created by AngzAngy on 2017/10/24.
//

#ifndef AALIVE_YUVCONVERTER_H
#define AALIVE_YUVCONVERTER_H
#include "Framebuffer.h"
#include "Texture2d.h"
#include "Shader.h"
class YuvConverter{
public:
    YuvConverter();
    ~YuvConverter();
    bool convert(void *buf, int width, int height, int srcTextureId);
private:
    void release();
    Shader* mShader;
    Framebuffer* mFramebuffer;
    Texture2d* mTexture2d;
    GLint vertexAttribLoc;
    GLint textureAttribLoc;
    GLint textureUniformLoc;
    GLint coeffsUniformLoc;
};
#endif //AALIVE_YUVCONVERTER_H

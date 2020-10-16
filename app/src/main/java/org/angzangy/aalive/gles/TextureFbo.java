
package org.angzangy.aalive.gles;

import android.opengl.GLES20;

public class TextureFbo {
    private int mFrameBufferId;
    private int mFboTextureId;
    private int mTextureTarget;
    private int mWidth;
    private int mHeight;

    public void createFbo(int textureWidth, int textureHeight, int textureTarget) {
        mWidth = textureWidth;
        mHeight = textureHeight;
        mTextureTarget=textureTarget;

        int[] framebuffers = new int[1];
        GLES20.glGenFramebuffers(1, framebuffers, 0);
        this.mFrameBufferId = framebuffers[0];

        int[] textures = new int[1];
        GLES20.glGenTextures(1, textures, 0);
        mFboTextureId = textures[0];

        GLES20.glBindTexture(textureTarget, mFboTextureId);
        GLES20.glTexParameterf(textureTarget, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_NEAREST);
        GLES20.glTexParameterf(textureTarget, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_NEAREST);
        GLES20.glTexParameteri(textureTarget, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(textureTarget, GLES20.GL_TEXTURE_WRAP_T,GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexImage2D(textureTarget, 0, GLES20.GL_RGBA, textureWidth, textureHeight, 0,
                GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, null);
    }

    public void bindFbo(int texture) {
        // framebuffer
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFrameBufferId);

        GLES20.glActiveTexture(texture);
        GLES20.glBindTexture(mTextureTarget, mFboTextureId);

        GLES20.glFramebufferTexture2D(GLES20.GL_FRAMEBUFFER, GLES20.GL_COLOR_ATTACHMENT0, mTextureTarget,
                mFboTextureId, 0);
    }

    public void unBindFbo(int texture) {
        GLES20.glActiveTexture(texture);
        GLES20.glBindTexture(mTextureTarget, 0);
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
    }

    public int getFrameBufferId() {
        return mFrameBufferId;
    }

    public int getTextureId() {
        return mFboTextureId;
    }

    public int getWidth(){
        return mWidth;
    }

    public int getHeight(){
        return mHeight;
    }

    public void delete(){
        int ids[] = new int[1];
        ids[0] = mFrameBufferId;
        GLES20.glDeleteFramebuffers(1, ids, 0);
        mFrameBufferId = 0;

        ids[0] = mFboTextureId;
        GLES20.glDeleteTextures(1, ids, 0);
        mFboTextureId = 0;
    }
}

package org.angzangy.aalive;

public class OffscreenSurface extends EglSurfaceBase {
    public OffscreenSurface(EglContext eglContext, int width, int height) {
        super(eglContext);
        createOffscreenSurface(width, height);
    }

    @Override
    public void releaseEglSurface() {
        super.releaseEglSurface();
        if(mEglContext != null) {
            mEglContext.release();
        }
    }
}

package org.angzangy.aalive;

import android.opengl.EGLContext;

import org.angzangy.aalive.gles.EGLContextWrapper;
import org.angzangy.aalive.gles.GLESEnvController;
import org.angzangy.aalive.gles.GlUtil;
import org.angzangy.aalive.gles.OnEGLContextStateChangeListener;
import org.angzangy.aalive.gles.OnTextureFboStateChangeListener;
import org.angzangy.aalive.gles.TextureFbo;

public class LiveTelecastController extends GLESEnvController implements OnEGLContextStateChangeListener, OnTextureFboStateChangeListener {
    private LiveTelecastNative liveTelecast;

    @Override
    public void onEGLContextCreated(EGLContextWrapper eglContext) {
        sendCreateEGLContextMsg(eglContext.getEglContext14());
    }

    @Override
    public void onTextureFboCreated(TextureFbo textureFbo) {
        sendSetSharedTextureFbo(textureFbo);
        sendCreateEGLSurface(textureFbo.getWidth(), textureFbo.getHeight());
    }

    @Override
    public void onTextureFboUpdated(TextureFbo textureFbo) {

    }

    @Override
    protected void createGLContext(EGLContext sharedEGLContext) {
        super.createGLContext(sharedEGLContext);
        liveTelecast = new LiveTelecastNative();
        liveTelecast.onPreviewSizeChanged(640, 480);
    }

    @Override
    protected void releaseGLContext() {
        super.releaseGLContext();
        if(liveTelecast != null) {
            liveTelecast.release();
            liveTelecast = null;
        }
    }

    @Override
    protected boolean onDrawFrame() {
        boolean ret = false;
        if(texture2DRender != null &&
                sharedTextureFbo != null) {
            texture2DRender.renderTexture2D(sharedTextureFbo.getTextureId(), GlUtil.IDENTITY_MATRIX, GlUtil.IDENTITY_MATRIX);
            ret = true;
        }

        if(liveTelecast != null && ret) {
            liveTelecast.readFbo(640, 480);
        }

        if(eglSurface != null) {
            eglSurface.swapBuffers();
        }
        return ret && (getRenderMode() == RENDERMODE_CONTINUOUSLY);
    }
}

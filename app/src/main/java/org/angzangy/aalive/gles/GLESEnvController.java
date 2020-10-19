package org.angzangy.aalive.gles;

import android.graphics.SurfaceTexture;
import android.opengl.EGLContext;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;

import org.angzangy.aalive.GlUtil;
import org.angzangy.aalive.LogPrinter;

public class GLESEnvController{
    /**
     * The renderer only renders
     * when the surface is created, or when {@link #sendFrameAvailableMsg} is called.
     */
    public final static int RENDERMODE_WHEN_DIRTY = 0;
    /**
     * The renderer is called
     * continuously to re-render
     */
    public final static int RENDERMODE_CONTINUOUSLY = 1;
    private HandlerThread handlerThread;
    private EnvHandler handler;
    private int renderMode = RENDERMODE_CONTINUOUSLY;
    protected TextureFbo sharedTextureFbo;
    protected EglContext eglContext;
    protected EglSurfaceBase eglSurface;
    protected Texture2DRenderer texture2DRender;

    public GLESEnvController() {
        handlerThread = new HandlerThread("GLESEnvControllerThread");
        handlerThread.start();
        handler = new EnvHandler(handlerThread.getLooper());
        handler.setGLESEnvController(this);
    }

    protected void createGLContext(EGLContext sharedEGLContext) {
        releaseGLContext();
        eglContext = new EglContext(sharedEGLContext,0);
        LogPrinter.e("createGLContext sharedEGLContext:"+sharedEGLContext+",eglContext:"+eglContext);
    }

    protected void createSurface(Object surface, int width, int height) {
        releaseGLSurface();
        LogPrinter.e("createSurface surface:"+surface+",w x h = "+width +" x "+height);
        if(surface == null) {
            eglSurface = new OffscreenSurface(eglContext, width, height);
            eglSurface.makeCurrent();
        } else {
            if(surface instanceof SurfaceTexture) {
                eglSurface = new WindowSurface(eglContext, (SurfaceTexture)surface);
                eglSurface.makeCurrent();
            } else if(surface instanceof Surface) {
                eglSurface = new WindowSurface(eglContext, (Surface)surface, false);
                eglSurface.makeCurrent();
            }
        }
        if(eglSurface != null) {
            texture2DRender = new Texture2DRenderer();
            texture2DRender.loadShader(Texture2DRenderer.VertexShader,
                    Texture2DRenderer.BASE_TEXTURE_FragmentShader);
        }
        LogPrinter.e("createSurface eglSurface:"+eglSurface);
    }

    protected void setSharedTextureFbo(TextureFbo sharedTextureFbo) {
        this.sharedTextureFbo = sharedTextureFbo;
    }

    public void setRenderMode(int renderMode) {
        this.renderMode = renderMode;
    }

    public int getRenderMode() {
        return renderMode;
    }

    public boolean isContinuouslyRender() {
        return renderMode == RENDERMODE_CONTINUOUSLY;
    }

    protected void releaseGLSurface() {
        if(texture2DRender != null) {
            texture2DRender.release();
            texture2DRender = null;
        }
        if(eglSurface != null) {
            eglSurface.releaseEglSurface();
            eglSurface = null;
        }
    }

    protected void releaseGLContext() {
        if(eglContext != null){
            eglContext.release();
        }
        eglContext = null;
    }

    protected void release() {
        releaseGLSurface();
        releaseGLContext();
        handlerThread.quitSafely();
    }

    protected boolean onDrawFrame() {
        boolean ret = false;
        if(texture2DRender != null &&
                sharedTextureFbo != null) {
            texture2DRender.renderTexture2D(sharedTextureFbo.getTextureId(), GlUtil.IDENTITY_MATRIX, GlUtil.IDENTITY_MATRIX);
            ret = true;
        }

        if(eglSurface != null && ret) {
            eglSurface.swapBuffers();
        }
        return isContinuouslyRender();
    }

    public void sendCreateEGLContextMsg(EGLContext sharedContext) {
        handler.obtainMessage(EnvHandler.MSG_CREATE_EGLCONTEXT, sharedContext).sendToTarget();
    }

    public void sendCreateEGLSurface(Object surface) {
        sendCreateEGLSurface(surface, 0, 0);
    }

    public void sendCreateEGLSurface(int width, int height) {
        sendCreateEGLSurface(null, width, height);
    }

    public void sendCreateEGLSurface(Object surface, int width, int height) {
        handler.obtainMessage(EnvHandler.MSG_CREATE_SURFACE,
                width, height, surface).sendToTarget();
    }

    public void sendSetSharedTextureFbo(TextureFbo sharedTextureFbo) {
        handler.obtainMessage(EnvHandler.MSG_SET_SHAREDFBO, sharedTextureFbo).sendToTarget();
    }

    public void sendFrameAvailableMsg() {
        handler.sendEmptyMessage(EnvHandler.MSG_DRAW_FRAME);
    }

    public void sendReleaseMsg() {
        handler.sendEmptyMessage(EnvHandler.MSG_SHUTDOWN);
    }

    public class EnvHandler extends Handler {
        public static final int MSG_CREATE_EGLCONTEXT = 0;
        public static final int MSG_CREATE_SURFACE = MSG_CREATE_EGLCONTEXT + 1;
        public static final int MSG_DRAW_FRAME = MSG_CREATE_EGLCONTEXT + 2;
        public static final int MSG_SHUTDOWN = MSG_CREATE_EGLCONTEXT + 3;
        public static final int MSG_SET_SHAREDFBO = MSG_CREATE_EGLCONTEXT + 4;
        private GLESEnvController controller;
        public EnvHandler(Looper looper) {
            super(looper);
        }

        public void setGLESEnvController(GLESEnvController envController) {
            controller = envController;
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_CREATE_EGLCONTEXT:
                    if(controller != null) {
                        controller.createGLContext((EGLContext) msg.obj);
                    }
                    break;
                case MSG_CREATE_SURFACE:
                    if(controller != null) {
                        controller.createSurface(msg.obj, msg.arg1, msg.arg2);
                    }
                    break;
                case MSG_SET_SHAREDFBO:
                    if(controller != null) {
                        controller.setSharedTextureFbo((TextureFbo) msg.obj);
                    }
                    break;
                case MSG_DRAW_FRAME:
                    if(controller != null) {
                        if (controller.onDrawFrame()) {
                            sendEmptyMessage(MSG_DRAW_FRAME);
                        }
                    }
                    break;
                case MSG_SHUTDOWN:
                    if(controller != null) {
                        controller.release();
                    }
                    break;
                default:
            }
        }
    }
}

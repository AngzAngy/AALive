package org.angzangy.aalive.gles;

import android.graphics.SurfaceTexture;
import android.opengl.EGLContext;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;

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
        Object [] wrapper = new Object[]{this, sharedContext};
        handler.obtainMessage(EnvHandler.MSG_CREATE_EGLCONTEXT, wrapper).sendToTarget();
    }

    public void sendCreateEGLSurface(Object surface) {
        sendCreateEGLSurface(surface, 0, 0);
    }

    public void sendCreateEGLSurface(int width, int height) {
        sendCreateEGLSurface(null, width, height);
    }

    public void sendCreateEGLSurface(Object surface, int width, int height) {
        Object [] wrapper = new Object[]{this, surface};
        handler.obtainMessage(EnvHandler.MSG_CREATE_SURFACE,
                width, height, wrapper).sendToTarget();
    }

    public void sendSetSharedTextureFbo(TextureFbo sharedTextureFbo) {
        Object [] wrapper = new Object[]{this, sharedTextureFbo};
        handler.obtainMessage(EnvHandler.MSG_SET_SHAREDFBO, wrapper).sendToTarget();
    }

    public void sendFrameAvailableMsg() {
        handler.obtainMessage(EnvHandler.MSG_DRAW_FRAME, this).sendToTarget();
    }

    public void sendReleaseMsg() {
        handler.obtainMessage(EnvHandler.MSG_SHUTDOWN, this).sendToTarget();
    }

    public void queueEvent(Runnable runnable, boolean isFront) {
        if(isFront) {
            handler.postAtFrontOfQueue(runnable);
        }else{
            handler.post(runnable);
        }
    }

    private static class EnvHandler extends Handler {
        public static final int MSG_CREATE_EGLCONTEXT = 0;
        public static final int MSG_CREATE_SURFACE = MSG_CREATE_EGLCONTEXT + 1;
        public static final int MSG_DRAW_FRAME = MSG_CREATE_EGLCONTEXT + 2;
        public static final int MSG_SHUTDOWN = MSG_CREATE_EGLCONTEXT + 3;
        public static final int MSG_SET_SHAREDFBO = MSG_CREATE_EGLCONTEXT + 4;
        public EnvHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_CREATE_EGLCONTEXT:
                    Object[] wrapper = (Object[]) msg.obj;
                    if(wrapper != null && wrapper.length == 2) {
                        ((GLESEnvController)wrapper[0]).createGLContext((EGLContext)wrapper[1]);
                    }
                    break;
                case MSG_CREATE_SURFACE:
                    wrapper = (Object[]) msg.obj;
                    if(wrapper != null && wrapper.length == 2) {
                        ((GLESEnvController)wrapper[0]).createSurface(wrapper[1], msg.arg1, msg.arg2);
                    }
                    break;
                case MSG_SET_SHAREDFBO:
                    wrapper = (Object[]) msg.obj;
                    if(wrapper != null && wrapper.length == 2) {
                        ((GLESEnvController)wrapper[0]).setSharedTextureFbo((TextureFbo)wrapper[1]);
                    }
                    break;
                case MSG_DRAW_FRAME:
                    if(msg.obj != null) {
                        if (((GLESEnvController)msg.obj).onDrawFrame()) {
                            obtainMessage(MSG_DRAW_FRAME, msg.obj).sendToTarget();
                        }
                    }
                    break;
                case MSG_SHUTDOWN:
                    if(msg.obj != null) {
                        ((GLESEnvController)msg.obj).release();
                    }
                    break;
                default:
            }
        }
    }
}

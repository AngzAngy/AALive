package org.angzangy.aalive;

import android.graphics.SurfaceTexture;
import android.opengl.EGLContext;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;

import org.angzangy.aalive.gles.EGLContextWrapper;
import org.angzangy.aalive.gles.EglContext;
import org.angzangy.aalive.gles.EglSurfaceBase;
import org.angzangy.aalive.gles.OffscreenSurface;
import org.angzangy.aalive.gles.OnEGLContextStateChangeListener;
import org.angzangy.aalive.gles.OnTextureFboStateChangeListener;
import org.angzangy.aalive.gles.Texture2DRenderer;
import org.angzangy.aalive.gles.TextureFbo;
import org.angzangy.aalive.gles.WindowSurface;

public class LiveTelecastController implements OnEGLContextStateChangeListener, OnTextureFboStateChangeListener {
    private HandlerThread handlerThread;
    private TelecastHandler handler;
    private static LiveTelecastController instance = new LiveTelecastController();

    public static LiveTelecastController getInstance() {
        return instance;
    }

    private LiveTelecastController () {
        handlerThread = new HandlerThread("LiveTelecastThread");
        handlerThread.start();
        handler = new TelecastHandler(handlerThread.getLooper());
    }

    @Override
    public void onEGLContextCreated(EGLContextWrapper eglContext) {
        handler.obtainMessage(TelecastHandler.INIT, eglContext.getEglContext14()).sendToTarget();
    }

    @Override
    public void onTextureFboCreated(TextureFbo textureFbo) {
        handler.sharedTextureFbo = textureFbo;
        handler.obtainMessage(TelecastHandler.CREATE_SURFACE,
                textureFbo.getWidth(), textureFbo.getHeight(), null).sendToTarget();
    }

    public void release() {
        handler.sendEmptyMessage(TelecastHandler.UNINIT);
    }

    public class TelecastHandler extends Handler {
        public static final int INIT = 0;
        public static final int CREATE_SURFACE = INIT + 1;
        public static final int TELECAST = INIT + 2;
        public static final int UNINIT = INIT + 3;
        public TextureFbo sharedTextureFbo;
        private LiveTelecastNative liveTelecast;
        private EglContext eglContext;
        private EglSurfaceBase eglSurface;
        private Texture2DRenderer texture2DRender;

        public TelecastHandler(Looper looper) {
            super(looper);
        }

        private void createGLContext(EGLContext sharedEGLContext) {
            releaseGLContext();
            liveTelecast = new LiveTelecastNative();
            liveTelecast.onPreviewSizeChanged(640, 480);
            eglContext = new EglContext(sharedEGLContext,0);
            LogPrinter.e("createGLContext sharedEGLContext:"+sharedEGLContext+",eglContext:"+eglContext);
        }

        private void createSurface(Object surface, int width, int height) {
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

        private void releaseGLSurface() {
            if(texture2DRender != null) {
                texture2DRender.release();
                texture2DRender = null;
            }
            if(eglSurface != null) {
                eglSurface.releaseEglSurface();
                eglSurface = null;
            }
        }

        private synchronized void releaseGLContext() {
            if(liveTelecast != null) {
                liveTelecast.release();
                liveTelecast = null;
            }
        }

        private boolean doTelecast() {
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
            return ret;
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case INIT:
                    createGLContext((EGLContext)msg.obj);
                    break;
                case CREATE_SURFACE:
                    createSurface(msg.obj, msg.arg1, msg.arg2);
                    sendEmptyMessage(TelecastHandler.TELECAST);
                    break;
                case TELECAST:
                    if(doTelecast()) {
                        sendEmptyMessage(TelecastHandler.TELECAST);
                    }
                    break;
                case UNINIT:
                    releaseGLSurface();
                    releaseGLContext();
                    break;
                default:
            }
        }
    }
}

package org.angzangy.aalive;

import android.opengl.EGLContext;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

public class LiveTelecastController implements SharedGLContextStateChangedListener {
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
    public void onSharedGLContext(EGLContext eglContext, TextureFbo textureFbo, Object sharedSync) {
        handler.sharedEGLContext = eglContext;
        handler.sharedTextureFbo = textureFbo;
        handler.sharedSyncObj = sharedSync;
        handler.sendEmptyMessage(TelecastHandler.INIT);
    }

    public void release() {
        handler.sendEmptyMessage(TelecastHandler.UNINIT);
    }

    public class TelecastHandler extends Handler {
        public static final int INIT = 0;
        public static final int TELECAST = 1;
        public static final int UNINIT = 2;
        public EGLContext sharedEGLContext;
        public TextureFbo sharedTextureFbo;
        public Object sharedSyncObj;
        private LiveTelecastNative liveTelecast;
        private OffscreenSurface offscreenSurface;
        private Texture2DRenderer texture2DRender;

        public TelecastHandler(Looper looper) {
            super(looper);
        }

        private void createGLContext() {
            releaseGLContext();
            liveTelecast = new LiveTelecastNative();
            liveTelecast.onPreviewSizeChanged(sharedTextureFbo.getWidth(), sharedTextureFbo.getHeight());
            offscreenSurface = new OffscreenSurface(new EglContext(sharedEGLContext,0), sharedTextureFbo.getWidth(), sharedTextureFbo.getHeight());
            offscreenSurface.makeCurrent();

            texture2DRender = new Texture2DRenderer();
            texture2DRender.loadShader(Texture2DRenderer.VertexShader,
                    Texture2DRenderer.BASE_TEXTURE_FragmentShader);
        }

        private synchronized void releaseGLContext() {
            if(liveTelecast != null) {
                liveTelecast.release();
                liveTelecast = null;
            }
            if(texture2DRender != null) {
                texture2DRender.release();
                texture2DRender = null;
            }
            if(offscreenSurface != null) {
                offscreenSurface.releaseEglSurface();
                offscreenSurface = null;
            }
        }

        private void notifyPreview() {
            if(sharedSyncObj != null) {
                synchronized (sharedSyncObj) {
                    sharedSyncObj.notify();
                }
            }
        }

        private boolean doTelecast() {
            boolean ret = false;
            if(texture2DRender != null &&
                    sharedTextureFbo != null) {
                texture2DRender.renderTexture2D(sharedTextureFbo.getTextureId(), GlUtil.IDENTITY_MATRIX, GlUtil.IDENTITY_MATRIX);
                ret = true;
            }

            notifyPreview();

            if(liveTelecast != null) {
                liveTelecast.readFbo(sharedTextureFbo.getWidth(), sharedTextureFbo.getHeight());
            }

            if(offscreenSurface != null) {
                offscreenSurface.swapBuffers();
            }
            return ret;
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case INIT:
                    createGLContext();
                    notifyPreview();
                    sendEmptyMessage(TelecastHandler.TELECAST);
                    break;
                case TELECAST:
                    if(doTelecast()) {
                        sendEmptyMessage(TelecastHandler.TELECAST);
                    }
                    break;
                case UNINIT:
                    releaseGLContext();
                    break;
                default:
            }
        }
    }
}

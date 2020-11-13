package org.angzangy.aalive;

import android.graphics.Rect;
import android.opengl.GLES20;

import org.angzangy.aalive.codec.AVCSurfaceEncoder;
import org.angzangy.aalive.codec.FrameReceiver;
import org.angzangy.aalive.gles.EGLContextWrapper;
import org.angzangy.aalive.gles.GLESEnvController;
import org.angzangy.aalive.gles.OnEGLContextStateChangeListener;

import java.io.IOException;

public class AVCSenderController extends GLESEnvController implements OnEGLContextStateChangeListener {
    private AVCSurfaceEncoder avcEncoder;
    private FrameReceiver frameReceiver;
    private Rect viewRect;
    private boolean recordingEnabled;
    private long frameIndex;
    protected int videoWidth, videoHeight;
    @Override
    public void onEGLContextCreated(EGLContextWrapper eglContext) {
        sendCreateEGLContextMsg(eglContext.getEglContext14());
    }

    public void setFrameReceiver(FrameReceiver frameReceiver) {
        this.frameReceiver = frameReceiver;
    }

    public void asyncStartEncoder() {
        if(recordingEnabled){
            return;
        }
        final int BIT_RATE = 4000000;   // 4Mbps
        try {
            if(avcEncoder == null) {
                avcEncoder = new AVCSurfaceEncoder(videoWidth, videoHeight, BIT_RATE, frameReceiver);
                sendCreateEGLSurface(avcEncoder.getInputSurface());
            }
            avcEncoder.asyncStart();
            recordingEnabled = true;
        }catch (IOException e){
            e.printStackTrace();
        }
    }

    public void asyncStopEncoder(){
        recordingEnabled = false;
       if(avcEncoder != null) {
           avcEncoder.asyncStop();
       }
    }

    public void asyncReleaseEncoder(){
        if(avcEncoder != null) {
            if(recordingEnabled) {
                asyncStopEncoder();
            }
            avcEncoder.asyncRelease();
        }
    }

    public void setVideoSize(int videoWidth, int videoHeight){
        this.videoWidth = videoWidth;
        this.videoHeight = videoHeight;
    }

    @Override
    public boolean isContinuouslyRender() {
        return recordingEnabled && super.isContinuouslyRender();
    }

    @Override
    protected boolean onDrawFrame() {
        if(recordingEnabled && eglSurface != null && avcEncoder != null
                && sharedTextureFbo != null) {
//            computeViewPort();
            eglSurface.makeCurrent();
            GLES20.glViewport(0, 0, videoWidth, videoHeight);
            GLES20.glClearColor(1.0f, 0, 0, 1);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

            avcEncoder.asyncFrameAvailable();
            texture2DRender.renderTexture2D(sharedTextureFbo.getTextureId(), GlUtil.IDENTITY_MATRIX, GlUtil.IDENTITY_MATRIX);
            eglSurface.setPresentationTime(computePresentationTimeNsec(frameIndex++, AVCSurfaceEncoder.getFrameRate()));
            eglSurface.swapBuffers();
            LogPrinter.d("AVCSenderController swapBuf");
        }
        return isContinuouslyRender();
    }

    private void computeViewPort() {
        if(viewRect == null) {
            viewRect = new Rect();
            int winWidth = sharedTextureFbo.getWidth();
            int winHeight = sharedTextureFbo.getHeight();
            float windowAspect = (float) winHeight / (float) winWidth;
            int outWidth, outHeight;
            if(videoHeight > windowAspect * videoWidth){
                outWidth = videoWidth;
                outHeight = (int)(windowAspect * videoWidth);
            } else {
                outWidth = (int)(videoHeight / windowAspect);
                outHeight = videoHeight;
            }
            int xoffset = (videoWidth - outWidth) / 2;
            int yoffset = (videoHeight - outHeight) / 2;
            viewRect.set(xoffset, yoffset, xoffset + outWidth, yoffset + outHeight);
        }

    }
    /**
     * Generates the presentation time for frame N, in nanoseconds.
     */
    private static long computePresentationTimeNsec(long frameIndex, int frameRate) {
        final long ONE_BILLION = 1000000000;
        return frameIndex * ONE_BILLION / frameRate;
    }

    protected void openStream(){
    }

    protected void closeStream(){
    }
}

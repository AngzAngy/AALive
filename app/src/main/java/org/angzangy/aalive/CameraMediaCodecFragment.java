package org.angzangy.aalive;

import android.content.Context;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraManager;
import android.opengl.GLES20;
import android.opengl.Matrix;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import org.angzangy.aalive.codec.AVCSurfaceEncoder;

import java.io.IOException;

public class CameraMediaCodecFragment extends BaseFragment implements SurfaceHolder.Callback,
    SurfaceTextureStateChangedListener{
    private ICameraDevices mCamera;
    private SurfaceView glSurfaceView;
    private SurfaceTexture mSurfaceTexture;
    private GLHandler glHandler;
    private FileReceiver fileReceiver;
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        return inflater.inflate(R.layout.camera_surfaceview_layout, null);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        fileReceiver = new FileReceiver(getActivity().getExternalCacheDir(), "aalive.h264");
        view.findViewById(R.id.bnt_start_live).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                fileReceiver.openOutputStream();
                if(glHandler != null){
                    glHandler.setRecordingEnabled(true);
                }
            }
        });
        view.findViewById(R.id.bnt_close_file).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                fileReceiver.closeOutputStream();
            }
        });
        glSurfaceView = ((SurfaceView) view.findViewById(R.id.gl_surface_view));
        glSurfaceView.getHolder().addCallback(this);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (!PermissionHelper.hasCameraPermission(getActivity())) {
            Toast.makeText(getActivity(),
                    "Camera permission is needed to run this application", Toast.LENGTH_LONG).show();
            PermissionHelper.launchPermissionSettings(getActivity());
        } else {
            openCamera(mSurfaceTexture, ICameraDevices.CAMERA_FACING_FRONT);
            setCameraParameter();
            startCameraPreview();
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        GLThread glThread = new GLThread();
        glThread.start();
        glHandler = new GLHandler(glThread.getLooper(), glThread);
        glThread.setOnFrameAvailableListener(glHandler);
        glThread.setSurfaceTextureStateChangedListener(this);
        glThread.setDataReceiver(fileReceiver);
        glHandler.sendSurfaceCreated(holder);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if(glHandler != null){
            glHandler.sendSurfaceChanged(width, height);
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if(glHandler != null){
            glHandler.sendShutdown();
        }
        stopCameraPreview();
        releaseCamera();
    }

    @Override
    public void onSurfaceTextureCreated(SurfaceTexture surfaceTexture) {
        mSurfaceTexture = surfaceTexture;
        try {
            if(PermissionHelper.hasCameraPermission(getActivity())) {
                openCamera(surfaceTexture, ICameraDevices.CAMERA_FACING_FRONT);
                setCameraParameter();
                startCameraPreview();
            } else {
                PermissionHelper.requestCameraPermission(this);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onSurfaceTextureDestroyed() {

    }

    private void setCameraParameter(){
        if(mCamera != null) {
            mCamera.setCameraParameters();
        }
    }

    private void startCameraPreview(){
//        boolean bMirror = false;
//        if (mCameraId == CameraInfo.CAMERA_FACING_FRONT) {
//            bMirror = true;
//        }
//            int displayRotation = CameraUtil.getDisplayRotation(this);
//            int displayOrientation = CameraUtil.getDisplayOrientation(displayRotation,mCameraId);
        if (mCamera != null) {
            mCamera.startCameraPreview();
        }
    }

    private void stopCameraPreview(){
        if(mCamera!=null){
            mCamera.stopCameraPreview();
        }
    }

    private void openCamera(SurfaceTexture surfaceTexture, int cameraFacing){
        if(surfaceTexture != null) {
            mCamera = new Camera2Devices((CameraManager) getActivity().getSystemService(Context.CAMERA_SERVICE));//CameraOneDevices();
            mCamera.openCamera(surfaceTexture, cameraFacing);
        }
    }

    private void releaseCamera(){
        if(mCamera!=null){
            mCamera.releaseCamera();
            mCamera.setOnCameraPreviewSizeChangeListener(null);
            mCamera=null;
        }
    }

    private static class GLHandler extends Handler implements SurfaceTexture.OnFrameAvailableListener{
        private static final int MSG_SURFACE_CREATED = 0;
        private static final int MSG_SURFACE_CHANGED = 1;
        private static final int MSG_DO_FRAME = 2;
        private static final int MSG_RECORDING_ENABLED = 3;
        private static final int MSG_RECORD_METHOD = 4;
        private static final int MSG_SHUTDOWN = 5;

        private GLThread glThread;

        public GLHandler(Looper looper, GLThread thread) {
            super(looper);
            glThread = thread;
        }
        /**
         * Sends the "surface created" message.
         * <p>
         * Call from UI thread.
         */
        public void sendSurfaceCreated(SurfaceHolder holder) {
            sendMessage(obtainMessage(MSG_SURFACE_CREATED, holder));
        }

        /**
         * Sends the "surface changed" message, forwarding what we got from the SurfaceHolder.
         * <p>
         * Call from UI thread.
         */
        public void sendSurfaceChanged(int width, int height) {
            sendMessage(obtainMessage(MSG_SURFACE_CHANGED, width, height));
        }

        /**
         * Sends the "do frame" message, forwarding the Choreographer event.
         * <p>
         * Call from UI thread.
         */
        public void sendDoFrame(long frameTimeNanos) {
            sendMessage(obtainMessage(MSG_DO_FRAME,
                    (int) (frameTimeNanos >> 32), (int) frameTimeNanos));
        }

        /**
         * Enable or disable recording.
         * <p>
         * Call from non-UI thread.
         */
        public void setRecordingEnabled(boolean enabled) {
            sendMessage(obtainMessage(MSG_RECORDING_ENABLED, enabled ? 1 : 0, 0));
        }

        /**
         * Sends the "shutdown" message, which tells the render thread to halt.
         * <p>
         * Call from UI thread.
         */
        public void sendShutdown() {
            sendMessage(obtainMessage(MSG_SHUTDOWN));
        }

        @Override
        public void onFrameAvailable(SurfaceTexture surfaceTexture) {
            sendDoFrame(0);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if(glThread == null){
                return;
            }
            switch (msg.what) {
                case MSG_SURFACE_CREATED:
                    glThread.surfaceCreated((SurfaceHolder)msg.obj);
                    break;
                case MSG_SURFACE_CHANGED:
                    glThread.surfaceChanged(msg.arg1, msg.arg2);
                    break;
                case MSG_DO_FRAME:
                    glThread.doFrame(0);
                    break;
                case MSG_RECORDING_ENABLED:
                    glThread.startEncoder();
                    break;
                case MSG_SHUTDOWN:
                    glThread.releaseGL();
                    glThread.quit();
                    break;
                default:
                    break;
            }
        }
    }
    private static class GLThread extends HandlerThread{
        private EglContext eglContext;
        private WindowSurface windowSurface;
        private static final int GL_TEXTURE_EXTERNAL_OES = 0x8D65;
        private SurfaceTexture mSurfaceTexture;
        private int mSurfaceTextId;
        private SurfaceTextureStateChangedListener mSurfaceTextureStateChangedListener;
        private SurfaceTexture.OnFrameAvailableListener mOnFrameAvailableListener;
        private SurfaceTextureRenderer mSurfaceRenderer;
        protected float[] mFboMVPMatrix = new float[16];
        private float [] mScreenMVPMatrix = new float[16];//render to screen model-view-project matrix
        private float[] mSTMatrix = new float[16];//texture transport matrix
        private TextureFbo mTextureFbo;
        private Texture2DRenderer mTexture2DRender;
        private AVCSurfaceEncoder avcSurfaceEncoder;
        private WindowSurface encoderInputSurface;
        private Rect viewRect = new Rect();
        private boolean recordingEnabled;
        private long frameIndex;
        private DataReceiver dataReceiver;
        public GLThread() {
            super("GLThread");
        }

        public void setOnFrameAvailableListener(SurfaceTexture.OnFrameAvailableListener listener){
            mOnFrameAvailableListener = listener;
        }

        public void setSurfaceTextureStateChangedListener(SurfaceTextureStateChangedListener listener){
            mSurfaceTextureStateChangedListener = listener;
        }

        public void setDataReceiver(DataReceiver receiver) {
            dataReceiver = receiver;
        }

        @Override
        protected void onLooperPrepared() {
            super.onLooperPrepared();

            eglContext = new EglContext(null,
                    EglContext.FLAG_RECORDABLE | EglContext.FLAG_TRY_GLES3);
        }

        public void surfaceCreated(SurfaceHolder holder) {
            windowSurface = new WindowSurface(eglContext, holder.getSurface(), false);
            windowSurface.makeCurrent();

            GLES20.glEnable(GLES20.GL_TEXTURE);

            int[] textures = new int[1];
            GLES20.glGenTextures(1, textures, 0);

            mSurfaceTextId = textures[0];
            GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, mSurfaceTextId);

            // Can't do mipmapping with camera source
            GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER,
                    GLES20.GL_NEAREST);
            GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER,
                    GLES20.GL_LINEAR);
            // Clamp to edge is the only option
            GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S,
                    GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T,
                    GLES20.GL_CLAMP_TO_EDGE);

            Matrix.setIdentityM(mFboMVPMatrix, 0);
            Matrix.rotateM(mFboMVPMatrix, 0, 270, 0, 0, 1);
            Matrix.scaleM(mFboMVPMatrix, 0, 1, -1, 1);
            Matrix.setIdentityM(mScreenMVPMatrix, 0);
            Matrix.rotateM(mScreenMVPMatrix, 0, 180, 0, 0, 1);
            Matrix.setIdentityM(mSTMatrix, 0);
            mSurfaceRenderer = new MosaicSurfaceTextureRenderer();
            mSurfaceRenderer.loadShader(SurfaceTextureRenderer.VertexShader,
                    MosaicSurfaceTextureRenderer.OES_MosaicFragmentShader);

            mTexture2DRender = new Texture2DRenderer();
            mTexture2DRender.loadShader(Texture2DRenderer.VertexShader,
                    Texture2DRenderer.BASE_TEXTURE_FragmentShader);
            /*
             * Create the SurfaceTexture that will feed this textureID, and pass it to the camera
             */
            mSurfaceTexture = new SurfaceTexture(mSurfaceTextId);
            if(mOnFrameAvailableListener != null) {
                mSurfaceTexture.setOnFrameAvailableListener(mOnFrameAvailableListener);
            }
            if(mSurfaceTextureStateChangedListener !=null){
                mSurfaceTextureStateChangedListener.onSurfaceTextureCreated(mSurfaceTexture);
            }
        }

        public void surfaceChanged(int width, int height) {
            LogPrinter.d("SurfaceChanged ("+width+" x "+height+" )");
            int fboWidth = width;
            int fboHeight = height;
            mTextureFbo = new TextureFbo();
            mTextureFbo.createFbo(fboWidth, fboHeight, GLES20.GL_TEXTURE_2D);

        }

        /**
         * Advance state and draw frame in response to a vsync event.
         */
        private void doFrame(long timeStampNanos) {
            mSurfaceTexture.updateTexImage();
            windowSurface.makeCurrent();
            GLES20.glViewport(0, 0, windowSurface.getWidth(), windowSurface.getHeight());
            GLES20.glClearColor(1.0f, 0, 0, 1);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
            if(mSurfaceRenderer != null){
                mTextureFbo.bindFbo(GLES20.GL_TEXTURE1);
                mSurfaceRenderer.renderTexture2D(mSurfaceTextId, mFboMVPMatrix, mSTMatrix);
                mTextureFbo.unBindFbo(GLES20.GL_TEXTURE1);
            }
            mTexture2DRender.renderTexture2D(mTextureFbo.getTextureId(), mScreenMVPMatrix, mSTMatrix);
            windowSurface.swapBuffers();

            if(recordingEnabled && encoderInputSurface != null && avcSurfaceEncoder != null){
                encoderInputSurface.makeCurrent();
                GLES20.glViewport(viewRect.left, viewRect.top, viewRect.width(), viewRect.height());
                avcSurfaceEncoder.asyncFrameAvailable();
                mTexture2DRender.renderTexture2D(mTextureFbo.getTextureId(), mScreenMVPMatrix, mSTMatrix);
                encoderInputSurface.setPresentationTime(computePresentationTimeNsec(frameIndex++, AVCSurfaceEncoder.getFrameRate()));
                encoderInputSurface.swapBuffers();
            }
        }

        public void startEncoder(){
            if(recordingEnabled){
                return;
            }
            final int BIT_RATE = 4000000;   // 4Mbps
            final int VIDEO_WIDTH = 1280;
            final int VIDEO_HEIGHT = 720;
            int winWidth = windowSurface.getWidth();
            int winHeight = windowSurface.getHeight();
            LogPrinter.d("winSurfaceSize:" +winWidth +" x "+winHeight);
            float windowAspect = (float) winHeight / (float) winWidth;
            int outWidth, outHeight;
            if(VIDEO_HEIGHT > windowAspect * VIDEO_WIDTH){
                outWidth = VIDEO_WIDTH;
                outHeight = (int)(windowAspect * VIDEO_WIDTH);
            } else {
                outWidth = (int)(VIDEO_HEIGHT / windowAspect);
                outHeight = VIDEO_HEIGHT;
            }
            int xoffset = (VIDEO_WIDTH - outWidth) / 2;
            int yoffset = (VIDEO_HEIGHT - outHeight) / 2;
            viewRect.set(xoffset, yoffset, xoffset + outWidth, yoffset + outHeight);
            try {
                avcSurfaceEncoder = new AVCSurfaceEncoder(VIDEO_WIDTH, VIDEO_HEIGHT, BIT_RATE, dataReceiver);
                encoderInputSurface = new WindowSurface(eglContext, avcSurfaceEncoder.getInputSurface(), true);
                avcSurfaceEncoder.asyncStart();
                recordingEnabled = true;
            }catch (IOException e){
                e.printStackTrace();
            }

        }
        /**
         * Generates the presentation time for frame N, in nanoseconds.
         */
        private static long computePresentationTimeNsec(long frameIndex, int frameRate) {
            final long ONE_BILLION = 1000000000;
            return frameIndex * ONE_BILLION / frameRate;
        }

        public void releaseGL(){
            if(avcSurfaceEncoder != null){
                avcSurfaceEncoder.asyncStop();
                avcSurfaceEncoder.asyncRelease();
            }
            if(mSurfaceRenderer != null){
                mSurfaceRenderer.release();
            }
            if(mTexture2DRender != null){
                mTexture2DRender.release();
            }
            if(mTextureFbo != null){
                mTextureFbo.delete();
            }
            if(windowSurface != null){
                windowSurface.release();
            }
            if(encoderInputSurface != null){
                encoderInputSurface.release();
            }
            if(eglContext != null){
                eglContext.release();
            }
        }
    }
}

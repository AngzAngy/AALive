package org.angzangy.aalive;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import java.io.IOException;
import java.util.ArrayList;

/**
 * Created on 2017/1/24.
 */

public class CameraOneDevices implements ICameraDevices,
        Camera.PreviewCallback{
    private static final String TAG = "CameraOneDevices";
    private int mCameraId;
    private Camera mCamera;
    private HandlerThread mHandlerThread;
    private CameraHandler mCameraHandler;
    private Camera.Size mOptimalPreviewSize;
    private ArrayList<byte[]> mPreviewBuffers = new ArrayList<byte[]>();
    private int mBufferIndex;
    private LiveTelecastNative mLiveTelecastNative;
    public CameraOneDevices(){
        mHandlerThread = new HandlerThread("camera-one-thread");
        mHandlerThread.start();
        mCameraHandler = new CameraHandler(mHandlerThread.getLooper());
    }

    @Override
    public void openCamera(int cameraId) {
        if(cameraId == CAMERA_FACING_FRONT) {
            mCameraId = Camera.CameraInfo.CAMERA_FACING_FRONT;
        }
        if(cameraId == CAMERA_FACING_BACK){
            mCameraId = Camera.CameraInfo.CAMERA_FACING_BACK;
        }
        if(mCamera != null){
            stopCameraPreview();
            releaseCamera();
        }
        try {
            mCamera=Camera.open(mCameraId);

            if(mLiveTelecastNative != null){
                mLiveTelecastNative.release();
            }
            mLiveTelecastNative = new LiveTelecastNative();
            android.util.Log.e("AALive", "ainit mLiveTelecastNative---3");
        }catch (Exception e){
            if(mCamera != null){
                stopCameraPreview();
                releaseCamera();
            }
            e.printStackTrace();
        }
    }

    @Override
    public void setCameraParameters(){
        if(mCamera != null) {
            Camera.Parameters parameters = mCamera.getParameters();
            mOptimalPreviewSize = mCamera.new Size(640, 480);
            parameters.setPreviewSize(mOptimalPreviewSize.width, mOptimalPreviewSize.height);
            mCamera.setParameters(parameters);

            Message msg = mCameraHandler.obtainMessage(CameraHandler.SET_PREVIEW_SIZE_WHAT);
            msg.arg1 = mOptimalPreviewSize.width;
            msg.arg2 = mOptimalPreviewSize.height;
            msg.sendToTarget();

            Log.e(TAG,parameters.flatten());
        }
    }

    @Override
    public void startCameraPreview() {
        if(mCamera!=null){
            setPreviewCallbackWithBuffer(this);
            mCamera.startPreview();
        }
    }

    @Override
    public void setCameraPreviewTexture(SurfaceTexture surfaceTexture) {
        try {
            if(mCamera!=null) {
                mCamera.setPreviewTexture(surfaceTexture);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void stopCameraPreview() {
        if(mCamera!=null){
            mCamera.stopPreview();
        }
    }

    @Override
    public void releaseCamera() {
        if(mLiveTelecastNative != null){
            mLiveTelecastNative.release();
        }
        if(mCamera!=null){
            mCamera.release();
            mCamera=null;
        }
    }

    @Override
    public Size getCameraPerviewSize(){
        if(mOptimalPreviewSize != null){
            return new Size(mOptimalPreviewSize.width, mOptimalPreviewSize.height);
        }
        return null;
    }

    private void setPreviewCallbackWithBuffer(Camera.PreviewCallback cb){
        if(mCamera!=null){
            addCallbackBuffer();
            mCamera.setPreviewCallbackWithBuffer(cb);
        }
    }

    private void addCallbackBuffer(){
        if(mPreviewBuffers.isEmpty()){
            mBufferIndex = 0;
            for(int i = 0; i < 2; i++){
                try {
                    mPreviewBuffers.add(new byte[mOptimalPreviewSize.width * mOptimalPreviewSize.height * 3 / 2]);
                }catch (Throwable throwable){
                }
            }
        }
        if (mCamera != null) {
                mCamera.addCallbackBuffer(mPreviewBuffers.get(mBufferIndex));
                mBufferIndex = (mBufferIndex + 1) % mPreviewBuffers.size();
        }
    }

    @Override
    public void onPreviewFrame(byte[] bytes, Camera camera){
        Message msg = mCameraHandler.obtainMessage(CameraHandler.ON_PREVIEW_FRAME_WHAT);
        msg.obj = bytes;
        msg.arg1 = mOptimalPreviewSize.width;
        msg.arg2 = mOptimalPreviewSize.height;
        msg.sendToTarget();
    }

    class CameraHandler extends Handler {
        public static final int SET_PREVIEW_SIZE_WHAT = 0;
        public static final int ON_PREVIEW_FRAME_WHAT = 1;
        private boolean mHasPreviewSize;
        public CameraHandler(){
            super();
        }
        public CameraHandler(Looper looper){
            super(looper);
        }
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what){
                case SET_PREVIEW_SIZE_WHAT:
                    if(mLiveTelecastNative != null){
                        android.util.Log.e("AALive", "ainit onPreviewSizeChanged---0");
                        mLiveTelecastNative.onPreviewSizeChanged(msg.arg1, msg.arg2);
                        android.util.Log.e("AALive", "ainit onPreviewSizeChanged---1");
                        mHasPreviewSize = true;
                    }
                    break;
                case ON_PREVIEW_FRAME_WHAT:
                    if(mHasPreviewSize){
                        byte []bytes = (byte[])msg.obj;
                        mLiveTelecastNative.pushNV21Buffer(bytes, msg.arg1, msg.arg2);
                    }
                    addCallbackBuffer();
                    break;
            }
        }
    }
}

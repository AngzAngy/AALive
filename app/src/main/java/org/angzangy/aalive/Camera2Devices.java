package org.angzangy.aalive;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.support.v4.app.ActivityCompat;
import android.util.Size;
import android.view.Surface;

import java.lang.reflect.Array;
import java.util.Arrays;

/**
 * Created by Administrator on 2017/3/2.
 */

public class Camera2Devices implements ICameraDevices {
    private CameraManager mCameraMgr;
    private HandlerThread mHandlerThread;
    private Handler mCameraHandler;
    private ImageReader mImageReader;
    private String mInternalCameraId;
    private CameraDevice mCameraDevice;
    private SurfaceTexture mSurfaceTexture;
    private CaptureRequest.Builder mPreviewRequestBuilder;
    private CaptureRequest mPreviewRequest;

    public Camera2Devices(CameraManager cameraManager) {
        mCameraMgr = cameraManager;
        mHandlerThread = new HandlerThread("camera-2-thread");
        mHandlerThread.start();
        mCameraHandler = new Handler(mHandlerThread.getLooper());
    }

    private void setUpCameraOutput(int cameraId) {
        int cameraFacing = 0;
        switch (cameraFacing) {
            case CAMERA_FACING_BACK:
                cameraFacing = CameraCharacteristics.LENS_FACING_FRONT;
                break;
            case CAMERA_FACING_FRONT:
                cameraFacing = CameraCharacteristics.LENS_FACING_BACK;
                break;
        }
        try {
            for (String internalId : mCameraMgr.getCameraIdList()) {
                CameraCharacteristics cameraCharacteristics = mCameraMgr.getCameraCharacteristics(internalId);
                if (cameraCharacteristics.get(CameraCharacteristics.LENS_FACING) == cameraFacing) {
                    StreamConfigurationMap map =
                            cameraCharacteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                    Size largestSize = CameraUtil.getLargestSize(map.getOutputSizes(ImageFormat.JPEG));
                    mImageReader = ImageReader.newInstance(largestSize.getWidth(), largestSize.getHeight(),
                            ImageFormat.JPEG, 2);
                    mImageReader.setOnImageAvailableListener(mOnImageAvailableListener, mCameraHandler);

                    mInternalCameraId = internalId;
                    return;
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (RuntimeException e) {
            e.printStackTrace();
        }
    }

    private void createCaptureSession(){
        if(mSurfaceTexture != null && mCameraDevice != null){
            try {
                mPreviewRequestBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                Surface surface = new Surface(mSurfaceTexture);
                mPreviewRequestBuilder.addTarget(surface);

                mCameraDevice.createCaptureSession(Arrays.asList(surface, mImageReader.getSurface()),
                        new CameraCaptureSession.StateCallback() {
                            @Override
                            public void onConfigured(CameraCaptureSession cameraCaptureSession) {
                                mPreviewRequest = mPreviewRequestBuilder.build();
                                try {
                                    cameraCaptureSession.setRepeatingRequest(mPreviewRequest, mCaptureCallback, mCameraHandler);
                                } catch (CameraAccessException e) {
                                    e.printStackTrace();
                                }
                            }

                            @Override
                            public void onConfigureFailed(CameraCaptureSession cameraCaptureSession) {

                            }
                        }, null);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }
    }
    @Override
    public void openCamera(SurfaceTexture surfaceTexture, int cameraId) {
        setUpCameraOutput(cameraId);
        try {
            mSurfaceTexture = surfaceTexture;
            mCameraMgr.openCamera(mInternalCameraId, mStateCallback, mCameraHandler);
        }catch (CameraAccessException e){
            e.printStackTrace();
        }catch (RuntimeException e){
            e.printStackTrace();
        }
    }

    @Override
    public void setCameraParameters() {

    }

    @Override
    public void startCameraPreview() {

    }

    @Override
    public void stopCameraPreview() {

    }

    @Override
    public void releaseCamera() {
        if(mCameraDevice != null) {
            mCameraDevice.close();
            mCameraDevice = null;
        }
    }

    private ImageReader.OnImageAvailableListener mOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader imageReader) {

        }
    };

    private CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(CameraDevice cameraDevice) {
            mCameraDevice = cameraDevice;
            if(mSurfaceTexture != null){
                createCaptureSession();
            }
        }

        @Override
        public void onDisconnected(CameraDevice cameraDevice) {
            cameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(CameraDevice cameraDevice, int i) {
            cameraDevice.close();
            mCameraDevice = null;
        }
    };

    private CameraCaptureSession.CaptureCallback mCaptureCallback = new CameraCaptureSession.CaptureCallback() {
        @Override
        public void onCaptureStarted(CameraCaptureSession session, CaptureRequest request, long timestamp, long frameNumber) {
            super.onCaptureStarted(session, request, timestamp, frameNumber);
        }

        @Override
        public void onCaptureProgressed(CameraCaptureSession session, CaptureRequest request, CaptureResult partialResult) {
            super.onCaptureProgressed(session, request, partialResult);
        }

        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request, TotalCaptureResult result) {
            super.onCaptureCompleted(session, request, result);
        }

        @Override
        public void onCaptureFailed(CameraCaptureSession session, CaptureRequest request, CaptureFailure failure) {
            super.onCaptureFailed(session, request, failure);
        }

        @Override
        public void onCaptureSequenceCompleted(CameraCaptureSession session, int sequenceId, long frameNumber) {
            super.onCaptureSequenceCompleted(session, sequenceId, frameNumber);
        }

        @Override
        public void onCaptureSequenceAborted(CameraCaptureSession session, int sequenceId) {
            super.onCaptureSequenceAborted(session, sequenceId);
        }

        @Override
        public void onCaptureBufferLost(CameraCaptureSession session, CaptureRequest request, Surface target, long frameNumber) {
            super.onCaptureBufferLost(session, request, target, frameNumber);
        }
    };
}

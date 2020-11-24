package org.angzangy.aalive;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.OrientationEventListener;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import org.angzangy.aalive.cameradevice.Camera2Devices;
import org.angzangy.aalive.cameradevice.CameraUtil;
import org.angzangy.aalive.cameradevice.ICameraDevices;
import org.angzangy.aalive.gles.SurfaceTextureStateChangedListener;
import org.angzangy.aalive.ui.CameraPreviewGLView;

public class CameraFragment extends BaseFragment
implements SurfaceTextureStateChangedListener, OnCameraPreviewSizeChangeListener{
    private static final String TAG = "MainActivity";
    private CameraPreviewGLView mCameraGLView;
    private ICameraDevices mCamera;

    private MyOrientationEventListener mOrientationEventListener;
    // The degrees of the device rotated clockwise from its natural orientation.
    private int mOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;
    private LiveTelecastController liveTelecastController;

    private class MyOrientationEventListener extends OrientationEventListener{
        public MyOrientationEventListener(Context context){
            super(context);
        }

        @Override
        public void onOrientationChanged(int orientation) {
            // We keep the last known orientation. So if the user first orient
            // the camera then point the camera to floor or sky, we still have
            // the correct orientation.
            if (orientation == ORIENTATION_UNKNOWN) {
                return;
            }
            mOrientation = CameraUtil.roundOrientation(orientation, mOrientation);
//            if(mCamera!=null){
//                Camera.getCameraInfo(mCameraId, mCameraInfo);
//                mPictureRotation = CameraUtil.getPictureRotation(mCameraId, mCameraInfo, mOrientation);
//                Parameters parameters=mCamera.getParameters();
//                parameters.setRotation(mPictureRotation);
//                mCamera.setParameters(parameters);
//                Log.d(TAG, "pictureRotation=="+mPictureRotation);
//            }
        }
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        return inflater.inflate(R.layout.camera_layout, null);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        liveTelecastController = new LiveTelecastController();
        view.findViewById(R.id.btn_switch_camera).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                switchCamera();
            }
        });
        mCameraGLView=(CameraPreviewGLView)view.findViewById(R.id.camera_preview);
        mCameraGLView.setSurfaceTextureStateChangedListener(this);
        mCameraGLView.setOnEGLContextStateChangeListener(liveTelecastController);
        mCameraGLView.setOnTextureFboStateChangeListener(liveTelecastController);
        mOrientationEventListener = new MyOrientationEventListener(getContext());
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        if(mCameraGLView != null) {
            mCameraGLView.setOnEGLContextStateChangeListener(null);
            mCameraGLView.setOnTextureFboStateChangeListener(null);
        }
        liveTelecastController.sendReleaseMsg();
    }

    @Override
    public void onSurfaceTextureCreated(SurfaceTexture surfaceTexture) {
        try {
            if(PermissionHelper.hasCameraPermission(getActivity())) {
                openCamera(ICameraDevices.CAMERA_FACING_FRONT);
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
    public void onSurfaceTextureDestroyed(){

    }

    @Override
    public void OnCameraPreviewSize(final int width, final int height) {
        if(mCameraGLView != null){
            mCameraGLView.setSurfaceTextureSize(width, height);
            mCameraGLView.post(new Runnable() {
                @Override
                public void run() {
                    mCameraGLView.setAspectRadio(width, height);
                }
            });
        }
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

    private void openCamera(int cameraFacing){
        if(mCameraGLView != null && mCameraGLView.getSurfaceTexture() != null) {
            mCamera = new Camera2Devices((CameraManager) getActivity().getSystemService(Context.CAMERA_SERVICE));//CameraOneDevices();
            mCamera.setOnCameraPreviewSizeChangeListener(this);
            mCamera.openCamera(mCameraGLView.getSurfaceTexture(), cameraFacing);
        }
    }

    private void releaseCamera(){
        if(mCamera!=null){
            mCamera.releaseCamera();
            mCamera.setOnCameraPreviewSizeChangeListener(null);
            mCamera=null;
        }
    }

    @Override
    public void onPause(){
        super.onPause();
        if(mOrientationEventListener != null) {
            mOrientationEventListener.disable();
        }
        stopCameraPreview();
        releaseCamera();
    }

    @Override
    public void onResume(){
        super.onResume();
        if(mOrientationEventListener != null) {
            mOrientationEventListener.enable();
        }
        if(mCameraGLView != null){
            mCameraGLView.onResume();
            if(mCamera == null){
                onSurfaceTextureCreated(mCameraGLView.getSurfaceTexture());
            }
        }
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
        if(mCameraGLView != null){
            mCameraGLView.release();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (!PermissionHelper.hasCameraPermission(getActivity())) {
            Toast.makeText(getActivity(),
                    "Camera permission is needed to run this application", Toast.LENGTH_LONG).show();
            PermissionHelper.launchPermissionSettings(getActivity());
        } else {
            openCamera(ICameraDevices.CAMERA_FACING_FRONT);
            setCameraParameter();
            startCameraPreview();
        }
    }

    private void switchCamera(){
        releaseCamera();
        openCamera(ICameraDevices.CAMERA_FACING_BACK);
        setCameraParameter();
        startCameraPreview();
    }
}

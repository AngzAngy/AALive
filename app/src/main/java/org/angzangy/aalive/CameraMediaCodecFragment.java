package org.angzangy.aalive;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;

import org.angzangy.aalive.gles.EGLContextWrapper;
import org.angzangy.aalive.gles.OnEGLContextStateChangeListener;
import org.angzangy.aalive.gles.OnTextureFboStateChangeListener;
import org.angzangy.aalive.gles.SurfaceTextureStateChangedListener;
import org.angzangy.aalive.gles.TextureFbo;
import org.angzangy.aalive.ui.CameraPreviewGLView;

public class CameraMediaCodecFragment extends BaseFragment implements SurfaceTextureStateChangedListener {
    private ICameraDevices mCamera;
    private CameraPreviewGLView glSurfaceView;
    private SurfaceTexture mSurfaceTexture;
    private AVCDumpFileController dumpFileController;
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
        createDumpController();
        view.findViewById(R.id.bnt_start_live).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                fileReceiver.openOutputStream();
                if(dumpFileController != null){
                    dumpFileController.setRecordingEnabled(true);
                }
            }
        });
        view.findViewById(R.id.bnt_close_file).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                fileReceiver.closeOutputStream();
            }
        });
        glSurfaceView = ((CameraPreviewGLView) view.findViewById(R.id.preview_view));
        glSurfaceView.setOnEGLContextStateChangeListener(new OnEGLContextStateChangeListener() {
            @Override
            public void onEGLContextCreated(EGLContextWrapper eglContext) {
                if(dumpFileController != null) {
                    dumpFileController.sendCreateEGLContextMsg(eglContext.getEglContext14());
                }
            }
        });
        glSurfaceView.setOnTextureFboStateChangeListener(new OnTextureFboStateChangeListener() {
            @Override
            public void onTextureFboCreated(TextureFbo textureFbo) {
                if(dumpFileController != null) {
                    dumpFileController.sendSetSharedTextureFbo(textureFbo);
                }
            }
        });
        glSurfaceView.setSurfaceTextureStateChangedListener(this);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        if(dumpFileController != null){
            dumpFileController.sendReleaseMsg();
        }
        stopCameraPreview();
        releaseCamera();
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

    public void createDumpController() {
        fileReceiver = new FileReceiver(getActivity().getExternalCacheDir(), "aalive.h264");
        dumpFileController = new AVCDumpFileController();
        dumpFileController.setFileReceiver(fileReceiver);
    }

    @Override
    public void onSurfaceTextureCreated(SurfaceTexture surfaceTexture) {
        mSurfaceTexture = surfaceTexture;
        try {
            if(PermissionHelper.hasCameraPermission(getActivity())) {
                openCamera(surfaceTexture, ICameraDevices.CAMERA_FACING_FRONT);
                setCameraParameter();
                startCameraPreview();
                if(dumpFileController != null) {
                    dumpFileController.startVideoEncoder(1280, 720);
                    dumpFileController.sendFrameAvailableMsg();
                }
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
            mCamera = null;
        }
    }
}

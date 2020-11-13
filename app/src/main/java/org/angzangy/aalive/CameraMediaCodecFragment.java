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

import java.io.File;

public class CameraMediaCodecFragment extends BaseFragment implements SurfaceTextureStateChangedListener,
        OnCameraPreviewSizeChangeListener{
    private ICameraDevices mCamera;
    private CameraPreviewGLView glSurfaceView;
    private SurfaceTexture mSurfaceTexture;
    private AVCSenderController avcSenderController;
    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        return inflater.inflate(R.layout.camera_surfaceview_layout, null);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        createController();
        view.findViewById(R.id.bnt_start_live).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(avcSenderController != null){
                    avcSenderController.openStream();
                }
            }
        });
        view.findViewById(R.id.bnt_close_file).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(avcSenderController != null) {
                    avcSenderController.closeStream();
                }
            }
        });
        glSurfaceView = ((CameraPreviewGLView) view.findViewById(R.id.preview_view));
        glSurfaceView.setOnEGLContextStateChangeListener(new OnEGLContextStateChangeListener() {
            @Override
            public void onEGLContextCreated(EGLContextWrapper eglContext) {
                if(avcSenderController != null) {
                    avcSenderController.sendCreateEGLContextMsg(eglContext.getEglContext14());
                }
            }
        });
        glSurfaceView.setOnTextureFboStateChangeListener(new OnTextureFboStateChangeListener() {
            @Override
            public void onTextureFboCreated(TextureFbo textureFbo) {
                if(avcSenderController != null) {
                    avcSenderController.sendSetSharedTextureFbo(textureFbo);
                }
            }

            @Override
            public void onTextureFboUpdated(TextureFbo textureFbo) {
                if(avcSenderController != null) {
                    avcSenderController.sendFrameAvailableMsg();
                }
            }
        });
        glSurfaceView.setSurfaceTextureStateChangedListener(this);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        if(avcSenderController != null){
            avcSenderController.sendReleaseMsg();
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

    public void createController() {
//        File file = new File(getActivity().getExternalCacheDir(), "aalive.flv");
//        avcSenderController = new AVCDumpFileController(file);
        avcSenderController = new AVCRtmpController("rtmp://172.17.71.63:1935/myapp/aalive");
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
            mCamera.setOnCameraPreviewSizeChangeListener(this);
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

    @Override
    public void OnCameraPreviewSize(int width, int height) {
        if(avcSenderController != null) {
            avcSenderController.setVideoSize(width, height);
            avcSenderController.sendFrameAvailableMsg();
        }
    }
}

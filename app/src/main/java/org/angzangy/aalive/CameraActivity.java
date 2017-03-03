package org.angzangy.aalive;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraManager;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.ActivityCompat;
import android.util.Size;
import android.view.OrientationEventListener;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;

public class CameraActivity extends BaseActivity
implements SurfaceTextureStateChangedListener{
    private static final String TAG = "CameraActivity";
//    public static final double PREVIEW_RATIO = 16.0 / 9.0;
//    private Camera mCamera;
    private CameraPreviewGLView mCameraGLView;
//    private int mCameraId;
//    private CameraInfo mCameraInfo = new CameraInfo();
//    private int mPictureRotation;
    private ICameraDevices mCamera;

    private MyOrientationEventListener mOrientationEventListener;
    // The degrees of the device rotated clockwise from its natural orientation.
    private int mOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;

    private Handler mMainHandler = new Handler();

    private class MyOrientationEventListener extends OrientationEventListener{
        public MyOrientationEventListener(Context context){
            super(context);
        }

        @Override
        public void onOrientationChanged(int orientation) {
            // We keep the last known orientation. So if the user first orient
            // the camera then point the camera to floor or sky, we still have
            // the correct orientation.
            if (orientation == ORIENTATION_UNKNOWN)
                return;
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
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        UiUtils.initialize(this);
        setContentView(R.layout.camera_main_layout);
        mCameraGLView=(CameraPreviewGLView)findViewById(R.id.camera_preview);
        mCameraGLView.setSurfaceTextureStateChangedListener(this);
        mOrientationEventListener = new MyOrientationEventListener(this);

    }

    private void resizePreview(int width, int height){
        float ratio = ((float)width)/((float)height);
        ViewGroup.LayoutParams lytParam = mCameraGLView.getLayoutParams();
        lytParam.width=UiUtils.screenWidth();
        lytParam.height=(int)(UiUtils.screenWidth()*ratio);
        mCameraGLView.setLayoutParams(lytParam);
    }

    @Override
    public void onSurfaceTextureCreated(SurfaceTexture surfaceTexture) {
        try {
            openCamera(surfaceTexture);
            setCameraParameter();
            startCameraPreview();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void onSurfaceTextureDestroyed(){

    }

    private void setCameraParameter(){
        mCamera.setCameraParameters();
    }

    private void startCameraPreview(){
//        boolean bMirror = false;
//        if (mCameraId == CameraInfo.CAMERA_FACING_FRONT) {
//            bMirror = true;
//        }
//            int displayRotation = CameraUtil.getDisplayRotation(this);
//            int displayOrientation = CameraUtil.getDisplayOrientation(displayRotation,mCameraId);

        if(mCamera!=null){
            mCamera.startCameraPreview();
        }
    }

    private void stopCameraPreview(){
        if(mCamera!=null){
            mCamera.stopCameraPreview();
        }
    }

    private void openCamera(SurfaceTexture surfaceTexture){
        if(mCamera != null){
            stopCameraPreview();
            releaseCamera();
        }
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            // TODO: Consider calling
            //    ActivityCompat#requestPermissions
            // here to request the missing permissions, and then overriding
            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
            //                                          int[] grantResults)
            // to handle the case where the user grants the permission. See the documentation
            // for ActivityCompat#requestPermissions for more details.
            return;
        }
        mCamera = new Camera2Devices((CameraManager) getSystemService(Context.CAMERA_SERVICE));//CameraOneDevices();
        mCamera.setOnPreviewSizeChangedListener(new ICameraDevices.OnPreviewSizeChangedListener() {
            @Override
            public void onPreviewSizeChanged(final int width, final int height) {
                mMainHandler.post(new Runnable(){
                    @Override
                    public void run() {
                        resizePreview(width, height);
                    }
                });
            }
        });
        mCamera.openCamera(surfaceTexture, ICameraDevices.CAMERA_FACING_FRONT);
    }

    private void releaseCamera(){
        if(mCamera!=null){
            mCamera.releaseCamera();
            mCamera=null;
        }
    }

    public void onPause(){
        super.onPause();
        mOrientationEventListener.disable();
        mCameraGLView.onPause();
        stopCameraPreview();
        releaseCamera();
    }

    public void onResume(){
        super.onResume();
        mOrientationEventListener.enable();
        mCameraGLView.onResume();
        if(mCameraGLView!=null && mCameraGLView.getSurfaceTexture()!=null){
            onSurfaceTextureCreated(mCameraGLView.getSurfaceTexture());
        }
    }

    protected void onDestroy(){
        super.onDestroy();
        stopCameraPreview();
        releaseCamera();
    }
}

package org.angzangy.aalive;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.util.SparseIntArray;
import android.view.OrientationEventListener;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;

public class CameraActivity extends BaseActivity
implements SurfaceTextureStateChangedListener{
    private static final String TAG = "CameraActivity";
    public static final double PREVIEW_RATIO = 16.0 / 9.0;
    private Camera mCamera;
    private CameraPreviewGLView mCameraGLView;
    private int mCameraId;
    private CameraInfo mCameraInfo = new CameraInfo();
    private int mPictureRotation;

    private MyOrientationEventListener mOrientationEventListener;
    // The degrees of the device rotated clockwise from its natural orientation.
    private int mOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;

    private Size mOptimalPreviewSize;
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
            if(mCamera!=null){
                Camera.getCameraInfo(mCameraId, mCameraInfo);
                mPictureRotation = CameraUtil.getPictureRotation(mCameraId, mCameraInfo, mOrientation);
                Parameters parameters=mCamera.getParameters();
                parameters.setRotation(mPictureRotation);
                mCamera.setParameters(parameters);
                Log.d(TAG, "pictureRotation=="+mPictureRotation);
            }
        }
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        UiUtils.initialize(this);
        mCameraId= CameraInfo.CAMERA_FACING_FRONT;
        setContentView(R.layout.camera_main_layout);
        mCameraGLView=(CameraPreviewGLView)findViewById(R.id.camera_preview);
        mCameraGLView.setSurfaceTextureStateChangedListener(this);
        mOrientationEventListener = new MyOrientationEventListener(this);

    }

    private void resizePreview(){
        float ratio = ((float)mOptimalPreviewSize.width)/((float)mOptimalPreviewSize.height);
        ViewGroup.LayoutParams lytParam = mCameraGLView.getLayoutParams();
        lytParam.width=UiUtils.screenWidth();
        lytParam.height=(int)(UiUtils.screenWidth()*ratio);
        mCameraGLView.setLayoutParams(lytParam);
    }

    @Override
    public void onSurfaceTextureCreated(SurfaceTexture surfaceTexture) {
        try {
            mCamera.setPreviewTexture(surfaceTexture);
            startCameraPreview();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void onSurfaceTextureDestroyed(){

    }

    private void setCameraParameter(){
        Parameters parameters=mCamera.getParameters();
        mOptimalPreviewSize = mCamera.new Size(1280,720);
        parameters.setPreviewSize(mOptimalPreviewSize.width, mOptimalPreviewSize.height);
        mCamera.setParameters(parameters);
        mMainHandler.post(new Runnable(){
            @Override
            public void run() {
                resizePreview();
            }
        });
        Log.i(TAG,parameters.flatten());
    }

    private void startCameraPreview(){
        boolean bMirror = false;
        if (mCameraId == CameraInfo.CAMERA_FACING_FRONT) {
            bMirror = true;
        }
            int displayRotation = CameraUtil.getDisplayRotation(this);
            int displayOrientation = CameraUtil.getDisplayOrientation(displayRotation,mCameraId);

        if(mCamera!=null){
            mCamera.startPreview();
        }
    }

    private void stopCameraPreview(){
        if(mCamera!=null){
            mCamera.stopPreview();
        }
    }

    private void openCamera(){
        if(mCamera != null){
            stopCameraPreview();
            releaseCamera();
        }
        mCamera=Camera.open(CameraInfo.CAMERA_FACING_BACK);
    }

    private void releaseCamera(){
        if(mCamera!=null){
            mCamera.release();
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
        openCamera();
        setCameraParameter();
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

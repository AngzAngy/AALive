package org.angzangy.aalive;

import android.graphics.SurfaceTexture;

/**
 * Created on 2017/1/24.
 */

public interface CameraDevices {
    public static final int CAMERA_FACING_BACK = 0;
    public static final int CAMERA_FACING_FRONT = 1;
    public void openCamera(int cameraId);
    public void setCameraParameters();
    public void startCameraPreview();
    public void setCameraPreviewTexture(SurfaceTexture surfaceTexture);
    public void stopCameraPreview();
    public void releaseCamera();
    public Size getCameraPerviewSize();
}

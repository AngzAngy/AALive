package org.angzangy.aalive;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.graphics.SurfaceTexture;

public class CameraPreviewGLView extends GLSurfaceView
implements SurfaceTexture.OnFrameAvailableListener{
    private CameraPreviewGLRender mRender;

    public CameraPreviewGLView(Context context) {
        super(context);
        init(context);
    }

    public CameraPreviewGLView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    /*
     * init
     * @see android.opengl.GLSurfaceView
     */
    private void init(Context context) {
        this.setEGLContextClientVersion(2);// @see android.opengl.GLSurfaceView
        this.mRender = new CameraPreviewGLRender();
        this.mRender.setOnFrameAvailableListener(this);
        this.setRenderer(this.mRender);//@see android.opengl.GLSurfaceView
        this.setZOrderMediaOverlay(true);//@see android.opengl.GLSurfaceView
        this.setRenderMode(RENDERMODE_WHEN_DIRTY);//@see android.opengl.GLSurfaceView
    }

    public SurfaceTexture getSurfaceTexture() {
        if(mRender != null){
            return mRender.getSurfaceTexture();
        }else{
            return null;
        }
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        requestRender();
    }

    public void setSurfaceTextureStateChangedListener(SurfaceTextureStateChangedListener listener){
        if(mRender != null){
            mRender.setmSurfaceTextureStateChangedListener(listener);
        }
    }

    public void release(){
        if(mRender != null){
            mRender.release();
        }
    }
}

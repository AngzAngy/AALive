package org.angzangy.aalive.ui;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.graphics.SurfaceTexture;

import org.angzangy.aalive.gles.OnTextureFboStateChangeListener;
import org.angzangy.aalive.gles.SurfaceTextureStateChangedListener;
import org.angzangy.aalive.gles.OnEGLContextStateChangeListener;

public class CameraPreviewGLView extends GLSurfaceView
implements SurfaceTexture.OnFrameAvailableListener{
    private int mRatioWidth;
    private int mRatioHeight;
    private CameraPreviewGLRender mRender;

    public CameraPreviewGLView(Context context) {
        super(context);
        init(context);
    }

    public CameraPreviewGLView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public void setAspectRadio(int ratioWidth, int ratioHeight){
        if (ratioWidth < 0 || ratioHeight < 0) {
            throw new IllegalArgumentException("Size cannot be negative.");
        }
        mRatioWidth = ratioWidth;
        mRatioHeight = ratioHeight;
        requestLayout();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec){
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        int width = MeasureSpec.getSize(widthMeasureSpec);
        int height = MeasureSpec.getSize(heightMeasureSpec);
        if(mRatioWidth <= 0 || mRatioHeight <= 0){
            setMeasuredDimension(width, height);
            return;
        }
        if(width < height * mRatioWidth / mRatioHeight){
            setMeasuredDimension(width, width * mRatioHeight / mRatioWidth);
        }else{
            setMeasuredDimension(height * mRatioWidth / mRatioHeight, height);
        }
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
            mRender.setSurfaceTextureStateChangedListener(listener);
        }
    }

    public void setOnEGLContextStateChangeListener(OnEGLContextStateChangeListener listener) {
        if(mRender != null) {
            mRender.setOnEGLContextStateChangeListener(listener);
        }
    }

    public void setOnTextureFboStateChangeListener(OnTextureFboStateChangeListener listener) {
        if(mRender != null){
            mRender.setOnTextureFboStateChangeListener(listener);
        }
    }

    public void setSurfaceTextureSize(int width, int height){
        if(mRender != null){
            mRender.setSurfaceTextureSize(width, height);
        }
    }

    public void release(){
        if(mRender != null){
            mRender.release();
        }
    }
}

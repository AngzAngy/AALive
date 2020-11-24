package org.angzangy.aalive.ui;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.graphics.SurfaceTexture;
import android.opengl.EGL14;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;

import org.angzangy.aalive.gles.GlUtil;
import org.angzangy.aalive.ImageSizeSurfaceTextureRenderer;
import org.angzangy.aalive.LogPrinter;
import org.angzangy.aalive.gles.OnTextureFboStateChangeListener;
import org.angzangy.aalive.gles.SurfaceTextureRenderer;
import org.angzangy.aalive.gles.SurfaceTextureStateChangedListener;
import org.angzangy.aalive.gles.Texture2DRenderer;
import org.angzangy.aalive.gles.TextureFbo;
import org.angzangy.aalive.gles.EGLContextWrapper;
import org.angzangy.aalive.gles.OnEGLContextStateChangeListener;

public class CameraPreviewGLRender implements GLSurfaceView.Renderer,
    SurfaceTexture.OnFrameAvailableListener{
    private static final int GL_TEXTURE_EXTERNAL_OES = 0x8D65;
    private SurfaceTexture mSurfaceTexture;
    private int mSurfaceTextureWidth;
    private int mSurfaceTextureHeight;
    private int mSurfaceTextId;
    private volatile boolean mUpdateTexImage = false;
    private SurfaceTextureStateChangedListener mSurfaceTextureStateChangedListener;
    private SurfaceTexture.OnFrameAvailableListener mOnFrameAvailableListener;
    private OnEGLContextStateChangeListener mOnEGLContextStateChangeListener;
    private OnTextureFboStateChangeListener mOnTextureFboStateChangeListener;
    private ImageSizeSurfaceTextureRenderer mSurfaceRenderer;
    protected float[] mFboMVPMatrix = new float[16];
//    private float [] mScreenMVPMatrix = new float[16];//render to screen model-view-project matrix
//    private float[] mSTMatrix = new float[16];//texture transport matrix
    private TextureFbo mTextureFbo;
    private Texture2DRenderer mTexture2DRender;

    /*
     * render loop, in render thread
     * @see android.opengl.GLSurfaceView
     * @see @see android.opengl.GLSurfaceView.Renderer
     */
    @Override
    public void onDrawFrame(GL10 glUnused) {
            if (mUpdateTexImage) {
                mSurfaceTexture.updateTexImage();
                mUpdateTexImage = false;
            }
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        if(mSurfaceRenderer != null){
            GLES20.glViewport(0, 0, mTextureFbo.getWidth(), mTextureFbo.getHeight());
            mTextureFbo.bindFbo(GLES20.GL_TEXTURE1);
            mSurfaceRenderer.renderTexture2D(mSurfaceTextId, mFboMVPMatrix, GlUtil.IDENTITY_MATRIX);
            mTextureFbo.unBindFbo(GLES20.GL_TEXTURE1);

            mTexture2DRender.renderTexture2D(mTextureFbo.getTextureId(), GlUtil.IDENTITY_MATRIX, GlUtil.IDENTITY_MATRIX);

            if(mOnTextureFboStateChangeListener != null) {
                mOnTextureFboStateChangeListener.onTextureFboUpdated(mTextureFbo);
            }
        }
    }

    /*
     * surface changed, in render thread
     *
     * @see android.opengl.GLSurfaceView
     * @see @see android.opengl.GLSurfaceView.Renderer
     */
    @Override
    public void onSurfaceChanged(GL10 glUnused, int width, int height) {
        LogPrinter.d("GLSurfaceChanged ("+width+" x "+height+" )");
        int fboWidth = width;
        int fboHeight = height;
        if(mSurfaceTextureWidth > 0 && mSurfaceTextureHeight > 0) {
            fboWidth = mSurfaceTextureWidth;
            fboHeight = mSurfaceTextureHeight;
        }
        LogPrinter.d("SurfaceTextureSize ("+mSurfaceTextureWidth+" x "+mSurfaceTextureHeight+" )");
        mSurfaceRenderer.setImageSize(fboWidth, fboHeight);

        if(mTextureFbo != null){
            mTextureFbo.delete();
        }else{
            mTextureFbo = new TextureFbo();
        }
        mTextureFbo.createFbo(fboWidth, fboHeight, GLES20.GL_TEXTURE_2D);

        if(mOnTextureFboStateChangeListener != null) {
            LogPrinter.e("mybug onSurfaceChanged eglGetCurrentContext:"+EGL14.eglGetCurrentContext());
            mOnTextureFboStateChangeListener.onTextureFboCreated(mTextureFbo);
        }
    }

    /*
     * surface created, in render thread
     *
     * @see android.opengl.GLSurfaceView
     * @see @see android.opengl.GLSurfaceView.Renderer
     */
    @Override
    public void onSurfaceCreated(GL10 glUnused, EGLConfig config) {
        if(mOnEGLContextStateChangeListener != null) {
            mOnEGLContextStateChangeListener.onEGLContextCreated(new EGLContextWrapper(null, EGL14.eglGetCurrentContext()));
        }

        GLES20.glEnable(GLES20.GL_TEXTURE);

        int[] textures = new int[1];
        GLES20.glGenTextures(1, textures, 0);

        mSurfaceTextId = textures[0];
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, mSurfaceTextId);

        // Can't do mipmapping with camera source
        GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER,
                GLES20.GL_NEAREST);
        GLES20.glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER,
                GLES20.GL_LINEAR);
        // Clamp to edge is the only option
        GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S,
                GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T,
                GLES20.GL_CLAMP_TO_EDGE);

        Matrix.setIdentityM(mFboMVPMatrix, 0);
        Matrix.rotateM(mFboMVPMatrix, 0, 90, 0, 0, 1);
//        Matrix.scaleM(mFboMVPMatrix, 0, 1, -1, 1);
//        Matrix.setIdentityM(mScreenMVPMatrix, 0);
//        Matrix.setIdentityM(mSTMatrix, 0);
        if(mSurfaceRenderer == null){
            mSurfaceRenderer = new ImageSizeSurfaceTextureRenderer();
        }
        mSurfaceRenderer.loadShader(SurfaceTextureRenderer.VertexShader,
                ImageSizeSurfaceTextureRenderer.OES_MosaicFragmentShader);

        mTexture2DRender = new Texture2DRenderer();
        mTexture2DRender.loadShader(Texture2DRenderer.VertexShader,
                Texture2DRenderer.BASE_TEXTURE_FragmentShader);
        /*
         * Create the SurfaceTexture that will feed this textureID, and pass it to the camera
         */
        if(mSurfaceTexture==null){
            mSurfaceTexture = new SurfaceTexture(mSurfaceTextId);
            mSurfaceTexture.setOnFrameAvailableListener(this);

            if(mSurfaceTextureStateChangedListener !=null){
                mSurfaceTextureStateChangedListener.onSurfaceTextureCreated(mSurfaceTexture);
            }
        }
    }

    public SurfaceTexture getSurfaceTexture() {
        return mSurfaceTexture;
    }

    public void release(){
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surface) {
        /* For simplicity, SurfaceTexture calls here when it has new
         * data available.  Call may come in from some random thread,
         * so let's be safe and use synchronize. No OpenGL calls can be done here.
         */
        mUpdateTexImage = true;
        if(mOnFrameAvailableListener != null){
            mOnFrameAvailableListener.onFrameAvailable(surface);
        }
    }


    public void setSurfaceTextureStateChangedListener(SurfaceTextureStateChangedListener  l) {
        mSurfaceTextureStateChangedListener = l;
    }

    public void setOnFrameAvailableListener(SurfaceTexture.OnFrameAvailableListener listener){
        mOnFrameAvailableListener = listener;
    }

    public void setSurfaceTextureSize(int width, int height){
        if (width < 0 || width < 0) {
            throw new IllegalArgumentException("Size cannot be negative.");
        }
        mSurfaceTextureWidth = width;
        mSurfaceTextureHeight = height;
    }

    public void setOnEGLContextStateChangeListener(OnEGLContextStateChangeListener listener) {
        mOnEGLContextStateChangeListener = listener;
    }

    public void setOnTextureFboStateChangeListener(OnTextureFboStateChangeListener listener) {
        mOnTextureFboStateChangeListener = listener;
    }
}

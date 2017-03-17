package org.angzangy.aalive;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.graphics.SurfaceTexture;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.Matrix;

public class CameraPreviewGLRender implements GLSurfaceView.Renderer,
    SurfaceTexture.OnFrameAvailableListener{
    private static final int GL_TEXTURE_EXTERNAL_OES = 0x8D65;
    private SurfaceTexture mSurfaceTexture;
    private int mSurfaceTextId;
    private boolean mUpdateSurface = false;
    private SurfaceTextureStateChangedListener mSurfaceTextureStateChangedListener;
    private SurfaceTexture.OnFrameAvailableListener mOnFrameAvailableListener;
    private SurfaceTextureRenderer mSurfaceRenderer;
    protected float[] mMVPMatrix = new float[16];
    private float [] mIdentiryMatrix = new float[16];
    private TextureFbo mTextureFbo;
    private int mWindowWidth;
    private int mWindowHeight;
    private LiveTelecastNative mLiveTelecastNative;
    private Texture2DRenderer mTexture2DRender;


    /*
     * render loop, in render thread
     * @see android.opengl.GLSurfaceView
     * @see @see android.opengl.GLSurfaceView.Renderer
     */
    @Override
    public void onDrawFrame(GL10 glUnused) {
        synchronized(this) {
            if (mUpdateSurface) {
                mSurfaceTexture.updateTexImage();
                mUpdateSurface = false;
            }
        }
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        if(mSurfaceRenderer != null){
            GLES20.glViewport(0, 0, mTextureFbo.getWidth(), mTextureFbo.getHeight());
            mTextureFbo.bindFbo(GLES20.GL_TEXTURE1);
            mSurfaceRenderer.drawTexture2D(mSurfaceTextId, mMVPMatrix);
            mLiveTelecastNative.readFbo(mTextureFbo.getWidth(), mTextureFbo.getHeight());
            mTextureFbo.unBindFbo(GLES20.GL_TEXTURE1);
        }
        GLES20.glViewport(0, 0, mWindowWidth, mWindowHeight);
//        mSurfaceRenderer.drawTexture2D(mSurfaceTextId, mMVPMatrix);
        mTexture2DRender.drawTexture2D(mTextureFbo.getTextureId(), mIdentiryMatrix);
    }

    /*
     * surface changed, in render thread
     *
     * @see android.opengl.GLSurfaceView
     * @see @see android.opengl.GLSurfaceView.Renderer
     */
    @Override
    public void onSurfaceChanged(GL10 glUnused, int width, int height) {
        mWindowWidth = width;
        mWindowHeight = height;
        if(mTextureFbo != null){
            mTextureFbo.delete();
        }else{
            mTextureFbo = new TextureFbo();
        }
        mTextureFbo.createFbo(640 / 2, 480 / 2, GLES20.GL_TEXTURE_2D);

        if(mLiveTelecastNative != null){
            mLiveTelecastNative.release();
        }
        mLiveTelecastNative = new LiveTelecastNative();
        mLiveTelecastNative.onPreviewSizeChanged(mTextureFbo.getWidth(), mTextureFbo.getHeight());
    }

    /*
     * surface created, in render thread
     *
     * @see android.opengl.GLSurfaceView
     * @see @see android.opengl.GLSurfaceView.Renderer
     */
    @Override
    public void onSurfaceCreated(GL10 glUnused, EGLConfig config) {

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

        Matrix.setIdentityM(mMVPMatrix, 0);
        Matrix.rotateM(mMVPMatrix, 0, 90, 0, 0, 1);
        Matrix.scaleM(mMVPMatrix, 0, 1, -1, 1);
        Matrix.setIdentityM(mIdentiryMatrix, 0);
        if(mSurfaceRenderer == null){
            mSurfaceRenderer = new SurfaceTextureRenderer();
        }
        mSurfaceRenderer.loadShader(SurfaceTextureRenderer.VertexShader,
                SurfaceTextureRenderer.OES_FragmentShader);

        mTexture2DRender = new Texture2DRenderer();
        mTexture2DRender.loadShader(Texture2DRenderer.VertexShader,
                Texture2DRenderer.BASE_TEXTURE_FragmentShader);
        /*
         * Create the SurfaceTexture that will feed this textureID, and pass it to the camera
         */
        if(mSurfaceTexture==null){
            mSurfaceTexture = new SurfaceTexture(mSurfaceTextId);
            mSurfaceTexture.setOnFrameAvailableListener(this);

            synchronized(this) {
                mUpdateSurface = false;
            }
            if(mSurfaceTextureStateChangedListener !=null){
                mSurfaceTextureStateChangedListener.onSurfaceTextureCreated(mSurfaceTexture);
            }
        }
    }

    public SurfaceTexture getSurfaceTexture() {
        return mSurfaceTexture;
    }

    public void onPause(){
        if(mSurfaceTexture!=null){
            mSurfaceTexture.release();
            if(mSurfaceTextureStateChangedListener !=null){
                mSurfaceTextureStateChangedListener.onSurfaceTextureDestroyed();
            }
        }
        mSurfaceTexture = null;
    }

    @Override
    synchronized public void onFrameAvailable(SurfaceTexture surface) {
        /* For simplicity, SurfaceTexture calls here when it has new
         * data available.  Call may come in from some random thread,
         * so let's be safe and use synchronize. No OpenGL calls can be done here.
         */
        mUpdateSurface = true;
        if(mOnFrameAvailableListener != null){
            mOnFrameAvailableListener.onFrameAvailable(surface);
        }
    }


    public void setmSurfaceTextureStateChangedListener(SurfaceTextureStateChangedListener  l) {
        mSurfaceTextureStateChangedListener = l;
    }

    public void setOnFrameAvailableListener(SurfaceTexture.OnFrameAvailableListener listener){
        mOnFrameAvailableListener = listener;
    }
}

package org.angzangy.aalive;

import java.nio.FloatBuffer;
import android.opengl.GLES20;
import android.opengl.Matrix;

public class Texture2DRenderer {
    protected static final int FLOAT_SIZE_BYTES = 4;
    protected static final int VERTICES_DATA_STRIDE_BYTES = 5 * FLOAT_SIZE_BYTES;
    protected static final int VERTICES_DATA_POS_OFFSET = 0;
    protected static final int VERTICES_DATA_UV_OFFSET = 3;

    protected static final float[] mVerticesData = {
            // X, Y, Z, U, V
            -1.0f, -1.0f, 0, 0.f, 0.f,
            1.0f, -1.0f, 0, 1.f, 0.f,
            -1.0f, 1.0f, 0, 0.f, 1.f,
            1.0f, 1.0f, 0, 1.f, 1.f,
    };

    public static final String VertexShader =
            "uniform mat4 uMVPMatrix;\n" +
            "uniform mat4 uSTMatrix;\n" +
            "attribute vec4 aPosition;\n" +
            "attribute vec4 aTextureCoord;\n" +
            "varying vec2 vTextureCoord;\n" +
            "void main() {\n" +
            "  gl_Position = uMVPMatrix * aPosition;\n" +
            "  vTextureCoord = (uSTMatrix * aTextureCoord).xy;\n" +
            "}\n";

    public final static String BASE_TEXTURE_FragmentShader =
            "precision mediump float;\n" +
                    "varying vec2 vTextureCoord;\n" +
                    "uniform sampler2D sTexture;\n" +
                    "void main() {\n" +
                    "  gl_FragColor = texture2D(sTexture, vTextureCoord);\n" +
                    "}\n";

    protected FloatBuffer mVertices;
    protected int mProgram;
    protected int muMVPMatrixHandle;
    protected int muSTMatrixHandle;
    protected int maPositionHandle;
    protected int maTextureHandle;
    protected int mSamplerHandle;

    public Texture2DRenderer() {
        mVertices = NioBufferUtil.allocByteBuffer(mVerticesData.length
                * FLOAT_SIZE_BYTES).asFloatBuffer();
        mVertices.put(mVerticesData).position(0);
    }

    public void onSurfaceCreated(){
    }

    public void onSurfaceChanged(int width, int height){
    }

    public void loadShader(String vertexShader, String fragmentShader) {
        release();
        mProgram = ProgramShaderUtil.createProgram(vertexShader, fragmentShader);
        if (mProgram == 0) {
            return;
        }

        GLES20.glUseProgram(mProgram);

        maPositionHandle = GLES20.glGetAttribLocation(mProgram, "aPosition");
        if (maPositionHandle == -1) {
            throw new RuntimeException("Could not get attrib location for aPosition");
        }
        maTextureHandle = GLES20.glGetAttribLocation(mProgram, "aTextureCoord");
        if (maTextureHandle == -1) {
            throw new RuntimeException("Could not get attrib location for aTextureCoord");
        }

        muMVPMatrixHandle = GLES20.glGetUniformLocation(mProgram, "uMVPMatrix");
        if (muMVPMatrixHandle == -1) {
            throw new RuntimeException("Could not get attrib location for uMVPMatrix");
        }

        muSTMatrixHandle = GLES20.glGetUniformLocation(mProgram, "uSTMatrix");
        if (muSTMatrixHandle == -1) {
            throw new RuntimeException("Could not get attrib location for uSTMatrix");
        }

        mSamplerHandle = GLES20.glGetUniformLocation(mProgram, "sTexture");
        if (mSamplerHandle == -1) {
            throw new RuntimeException("Could not get attrib location for sTexture");
        }
    }

    public void renderTexture2D(int texture2DId, float[]mvpMatrix, float[]stMatrix){
        GLES20.glUseProgram(mProgram);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture2DId);

        setRendererParamaters(mvpMatrix, stMatrix);

        mVertices.position(VERTICES_DATA_POS_OFFSET);
        GLES20.glVertexAttribPointer(maPositionHandle, 3, GLES20.GL_FLOAT, false,
                VERTICES_DATA_STRIDE_BYTES, mVertices);
        GLES20.glEnableVertexAttribArray(maPositionHandle);

        mVertices.position(VERTICES_DATA_UV_OFFSET);
        GLES20.glVertexAttribPointer(maTextureHandle, 3, GLES20.GL_FLOAT, false,
                VERTICES_DATA_STRIDE_BYTES, mVertices);
        GLES20.glEnableVertexAttribArray(maTextureHandle);

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
    }

    public void release(){
        if(mProgram>0){
            GLES20.glDeleteProgram(mProgram);
            mProgram = 0;
        }
    }

    protected void setRendererParamaters(float[]mvpMatrix, float[]stMatrix){
        GLES20.glUniform1i(mSamplerHandle, 0);
        GLES20.glUniformMatrix4fv(muMVPMatrixHandle, 1, false, mvpMatrix, 0);
        GLES20.glUniformMatrix4fv(muSTMatrixHandle, 1, false, stMatrix, 0);
    }
}

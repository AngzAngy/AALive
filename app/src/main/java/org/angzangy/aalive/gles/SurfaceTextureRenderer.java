package org.angzangy.aalive.gles;

import android.opengl.GLES20;

public class SurfaceTextureRenderer extends Texture2DRenderer{
    private static final int GL_TEXTURE_EXTERNAL_OES = 0x8D65;

    public final  static String OES_FragmentShader =
            "#extension GL_OES_EGL_image_external : require\n" +
            "precision mediump float;\n" +
            "varying vec2 vTextureCoord;\n" +
            "uniform samplerExternalOES sTexture;\n" +
            "void main() {\n" +
            "  gl_FragColor = texture2D(sTexture, vTextureCoord);\n" +
            "}\n";

    public final  static String OES_ReflectionFragmentShader =
            "#extension GL_OES_EGL_image_external : require\n" +
                    "precision mediump float;\n" +
                    "varying vec2 vTextureCoord;\n" +
                    "uniform samplerExternalOES sTexture;\n" +
                    "void main(void) {\n" +
                        "vec2 normCoord = vTextureCoord;\n"+
                        "float u,v;\n"+
                        "if(normCoord.x < 0.5 && normCoord.y < 0.5){\n"+
                            "normCoord = vTextureCoord * 2.0;\n"+
                            "u = 0.4246; v=0.5753;\n"+
                        "}else if(normCoord.x > 0.5 && normCoord.y < 0.5){\n"+
                            "normCoord.x = (vTextureCoord.x -0.5) * 2.0 ;\n"+
                            "normCoord.y = vTextureCoord.y  * 2.0 ;\n"+
                            "u = 0.2870; v = 0.3678;\n"+
                        "}else if(normCoord.x < 0.5 && normCoord.y > 0.5){\n"+
                            "normCoord.x = vTextureCoord.x * 2.0 ;\n"+
                            "normCoord.y = (vTextureCoord.y - 0.5) * 2.0 ;\n"+
                            "u = 0.5671; v = 0.8282;\n"+
                        "}else {\n"+
                            "normCoord = (vTextureCoord - 0.5) * 2.0;\n"+
                            "u = 0.5845; v = 0.4362;\n"+
                        "}\n"+
                        "vec4 color = texture2D(sTexture, normCoord);\n"+
                        "float gray = 0.3*color.r + 0.59*color.g + 0.11*color.b;\n"+
                        "if(vTextureCoord.x < 0.5 && vTextureCoord.y > 0.5){\n"+
                        "}else {\n"+
                            "v -= 0.5; u -= 0.5;\n"+
                            "color.r = gray + 1.4075 * v;\n"+
                            "color.g = gray - 0.3455 * u - 0.7169* v;\n"+
                            "color.b = gray + 1.779*u;\n"+
                        "}\n"+
                        "gl_FragColor = color;\n"+
                    "}\n";

    public SurfaceTextureRenderer() {
        super();
    }

    @Override
    public void renderTexture2D(int texture2DId, float[] mvpMatrix, float[] stMatrix){
        GLES20.glUseProgram(mProgram);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture2DId);

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
        GLES20.glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    }

    @Override
    protected void setRendererParamaters(float[]mvpMatrix, float[]stMatrix){
        GLES20.glUniformMatrix4fv(muMVPMatrixHandle, 1, false, mvpMatrix, 0);
        GLES20.glUniformMatrix4fv(muSTMatrixHandle, 1, false, stMatrix, 0);
        GLES20.glUniform1i(mSamplerHandle, 0);
    }
}

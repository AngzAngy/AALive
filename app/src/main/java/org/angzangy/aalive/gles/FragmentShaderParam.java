package org.angzangy.aalive.gles;

import android.opengl.GLES20;

public class FragmentShaderParam {
      public final static String BASE_TEXTURE_FragmentShader =
      "precision mediump float;\n" +
      "varying vec2 vTextureCoord;\n" +
      "uniform sampler2D sTexture;\n" +
      "void main() {\n" +
      "  gl_FragColor = texture2D(sTexture, vTextureCoord);\n" +
      "}\n";

    private int mSamplerHandle;

    public boolean loadParameters(int program) {
        if (program == 0) {
            return false;
        }

        GLES20.glUseProgram(program);
        mSamplerHandle = GLES20.glGetUniformLocation(program, "sTexture");
        if (mSamplerHandle == -1) {
            throw new RuntimeException("Could not get attrib location for sTexture");
        }
        return true;
    }

    public void render(){
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glUniform1i(mSamplerHandle, 0);
    }
}

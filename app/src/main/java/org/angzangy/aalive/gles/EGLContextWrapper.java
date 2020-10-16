package org.angzangy.aalive.gles;

public class EGLContextWrapper {
    private javax.microedition.khronos.egl.EGLContext eglContext10;
    private android.opengl.EGLContext eglContext14;
    public EGLContextWrapper(javax.microedition.khronos.egl.EGLContext eglContext10, android.opengl.EGLContext eglContext14) {
        this.eglContext10 = eglContext10;
        this.eglContext14 = eglContext14;
    }

    public javax.microedition.khronos.egl.EGLContext getEglContext10() {
        return eglContext10;
    }

    public android.opengl.EGLContext getEglContext14() {
        return eglContext14;
    }
}

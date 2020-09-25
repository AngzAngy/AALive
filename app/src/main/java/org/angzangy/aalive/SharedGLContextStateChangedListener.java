package org.angzangy.aalive;

import android.opengl.EGLContext;

public interface SharedGLContextStateChangedListener {
    public void onSharedGLContext(EGLContext eglContext, TextureFbo textureFbo, Object sharedSync);
}

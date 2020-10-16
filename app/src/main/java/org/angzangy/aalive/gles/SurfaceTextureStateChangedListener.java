package org.angzangy.aalive.gles;

import android.graphics.SurfaceTexture;

/**
 * Created on 2017/1/14.
 */

public interface SurfaceTextureStateChangedListener {
    public void onSurfaceTextureCreated(SurfaceTexture surfaceTexture);
    public void onSurfaceTextureDestroyed();
}

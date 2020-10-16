package org.angzangy.aalive.gles;

import org.angzangy.aalive.gles.EglContext;
import org.angzangy.aalive.gles.EglSurfaceBase;

public class OffscreenSurface extends EglSurfaceBase {
    public OffscreenSurface(EglContext eglContext, int width, int height) {
        super(eglContext);
        createOffscreenSurface(width, height);
    }
}

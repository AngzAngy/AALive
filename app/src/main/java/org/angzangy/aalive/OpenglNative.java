package org.angzangy.aalive;

/**
 * Created on 2017/1/18.
 */

public class OpenglNative {
    static {
        try {
            System.loadLibrary("aalive");
        }catch (Throwable e){
            e.printStackTrace();
        }
    }
    private long nativeObj;
    public OpenglNative(){
        init();
    }
    private native void init();
    public native void release();

    public native void onSurfaceCreated();
    public native void onSurfaceChanged(int width, int height);
    public native void onDrawFrame();
    public native void pushNV21Buffer(byte[] buffer, int with, int height);
}

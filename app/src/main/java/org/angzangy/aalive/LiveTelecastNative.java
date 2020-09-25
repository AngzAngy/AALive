package org.angzangy.aalive;

import java.nio.ByteBuffer;

/**
 * Created on 2017/1/18.
 */

public class LiveTelecastNative {
    private final static String RTMP_URL = "rtmp://172.17.71.63:1935/myapp/aalive";
    static {
        try {
            System.loadLibrary("aalive");
        }catch (Throwable e){
            e.printStackTrace();
        }
    }
    private long nativeObj;
    public LiveTelecastNative(){
        init();
        setRtmpUrl(RTMP_URL);
    }
    private native void init();
    public native void release();

    public native void setRtmpUrl(String url);
    public native void onPreviewSizeChanged(int width, int height);
    public native void readFbo(int width, int height);
    public native void pushNV21Buffer(byte[] buffer, int with, int height);
    public native void pushNV21Buffer(ByteBuffer ybuffer, ByteBuffer vubuffer,
                                      int with, int height);
    public native void pushTexture(int texture, int width, int height);
}

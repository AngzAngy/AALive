package org.angzangy.aalive.muxer;

import java.nio.ByteBuffer;

public class RtmpMuxer {
    static {
        try {
            System.loadLibrary("aalive");
        }catch (Throwable e){
            e.printStackTrace();
        }
    }
    private long nativeObj;
    public RtmpMuxer(){
        nativeObj = init();
    }

    private native long init();

    public int open(String url, int videoWidth, int videoHeight) {
        return open(nativeObj, url, videoWidth, videoHeight);
    }
    private native int open(long nativeObj, String url, int videoWidth, int videoHeight);

    /**
     * write h264 nal units
     * @param data
     * @param offset
     * @param length
     * @param timestamp
     * @return 0 if it writes network successfully
     * -1 if it could not write
     */
    public int writeVideo(byte[] data, int offset, int length, long timestamp){
        return writeVideo(nativeObj, data, offset, length, timestamp);
    }
    public int writeVideo(ByteBuffer byteBuffer, int offsetInBytes, int sizeInBytes, long timestamp){
        return writeNioVideo(nativeObj, byteBuffer, offsetInBytes, sizeInBytes, timestamp);
    }
    private native int writeVideo(long nativeObj, byte[] data, int offset, int length, long timestamp);
    private native int writeNioVideo(long nativeObj,
                                     ByteBuffer byteBuffer, int offsetInBytes, int sizeInBytes,
                                     long timestamp);

    /**
     * Write raw aac data
     * @param data
     * @param offset
     * @param length
     * @param timestamp
     * @return 0 if it writes network successfully
     * -1 if it could not write
     */
    public int writeAudio(byte[] data, int offset, int length, long timestamp){
        return writeAudio(nativeObj, data, offset, length, timestamp);
    }
    public int writeAudio(ByteBuffer buffer, int offsetInBytes, int sizeInBytes, long timestamp){
        return writeNioAudio(nativeObj, buffer, offsetInBytes, sizeInBytes, timestamp);
    }
    private native int writeAudio(long nativeObj, byte[] data, int offset, int length, long timestamp);
    private native int writeNioAudio(long nativeObj,
                                     ByteBuffer byteBuffer, int offsetInBytes, int sizeInBytes,
                                     long timestamp);


    public int read(byte[] data, int offset, int size) {
        return read(nativeObj, data, offset, size);
    }
    private native int read(long nativeObj, byte[] data, int offset, int size);

    public int close() {
        return close(nativeObj);
    }
    private native int close(long nativeObj);

    public void writeFlvHeader(boolean isHaveAudio, boolean isHaveVideo) {
        writeFlvHeader(nativeObj, isHaveAudio, isHaveVideo);
    }
    private native void writeFlvHeader(long nativeObj, boolean isHaveAudio, boolean isHaveVideo);

    public void fileOpen(String filename) {
        fileOpen(nativeObj, filename);
    }
    private native void fileOpen(long nativeObj, String filename);

    public void fileClose() {
        fileClose(nativeObj);
    }
    private native void fileClose(long nativeObj);

    /**
     *
     * @return 1 if it is connected
     * 0 if it is not connected
     */
    public boolean isConnected() {
        return isConnected(nativeObj);
    }
    private native boolean isConnected(long nativeObj);

    public void release() {
        release(nativeObj);
        nativeObj = 0;
    }
    private native void release(long nativeObj);
}

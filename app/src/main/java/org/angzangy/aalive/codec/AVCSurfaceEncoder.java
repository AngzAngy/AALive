package org.angzangy.aalive.codec;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.Surface;

import org.angzangy.aalive.LogPrinter;

import java.io.IOException;
import java.nio.ByteBuffer;

public class AVCSurfaceEncoder{
    private static final String MIME_TYPE = "video/avc";    // H.264 Advanced Video Coding
    private static final int FRAME_RATE = 30;
    private static final int IFRAME_INTERVAL = 5;           // 5 seconds between I-frames

    private static final int MSG_START = 0;
    private static final int MSG_STOP = 1;
    private static final int MSG_RELEASE = 2;
    private static final int MSG_FRAME_AVAILABLE = 3;

    private HandlerThread encodeThread;
    private EncodeHanlder encodeHandler;
    private MediaCodec mediaCodec;
    private Surface inputSurface;
    private MediaCodec.BufferInfo bufferInfo;
    private ByteBuffer configBuffer;
    private long startTime;
    private FrameReceiver frameReceiver;

    public AVCSurfaceEncoder(int width, int height, int bitRate, FrameReceiver receiver) throws IOException{
        frameReceiver = receiver;
        MediaFormat mediaFormat = MediaFormat.createVideoFormat(MIME_TYPE, width, height);
        mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, bitRate);
        mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, getFrameRate());
        mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, IFRAME_INTERVAL);

        mediaCodec = MediaCodec.createEncoderByType(MIME_TYPE);
        mediaCodec.configure(mediaFormat, null, null,
                MediaCodec.CONFIGURE_FLAG_ENCODE);

        inputSurface = mediaCodec.createInputSurface();

        bufferInfo = new MediaCodec.BufferInfo();

        encodeThread = new HandlerThread("AVCSurfaceEncoder");
        encodeThread.start();
        encodeHandler = new EncodeHanlder(encodeThread.getLooper());
    }

    public Surface getInputSurface() {
        return inputSurface;
    }

    public static int getFrameRate() {
        return FRAME_RATE;
    }

    public void asyncStart(){
        encodeHandler.sendMessage(encodeHandler.obtainMessage(MSG_START, mediaCodec));
    }

    public void asyncStop(){
        encodeHandler.sendMessage(encodeHandler.obtainMessage(MSG_STOP, this));
    }

    public void asyncRelease() {
        encodeHandler.sendMessage(encodeHandler.obtainMessage(MSG_RELEASE, mediaCodec));
    }

    public void asyncFrameAvailable(){
        encodeHandler.sendMessage(encodeHandler.obtainMessage(MSG_FRAME_AVAILABLE, 0, 0, this));
    }

    private void stop(){
        if(mediaCodec == null){
            return;
        }
        drainEncoder(true);
        mediaCodec.stop();
    }

    private void drainEncoder(boolean endOfStream) {
        final int TIMEOUT_USEC = 10000;

        if(mediaCodec == null){
            return;
        }
        if (endOfStream) {
            mediaCodec.signalEndOfInputStream();
        }

        ByteBuffer[] encoderOutputBuffers = mediaCodec.getOutputBuffers();
        while (true) {
            int encoderStatus = mediaCodec.dequeueOutputBuffer(bufferInfo, TIMEOUT_USEC);
            if (encoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {
                // no output available yet
                if (!endOfStream) {
                    break;      // out of while
                }
            } else if (encoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                // not expected for an encoder
                encoderOutputBuffers = mediaCodec.getOutputBuffers();
            } else if (encoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                // should happen before receiving buffers, and should only happen once
            } else if (encoderStatus >= 0) {
                if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    if (!endOfStream) {
                        LogPrinter.d("reached end of stream unexpectedly");
                    } else {
                        LogPrinter.d("end of stream reached");
                    }
                    mediaCodec.releaseOutputBuffer(encoderStatus, false);
                    break;      // out of while
                }

                ByteBuffer encodedData = encoderOutputBuffers[encoderStatus];
                if (encodedData == null) {
                    throw new RuntimeException("encoderOutputBuffer " + encoderStatus +
                            " was null");
                }
                if(startTime == 0){
                    startTime = bufferInfo.presentationTimeUs / 1000;
                }
                if(frameReceiver != null) {
                    frameReceiver.receive(encodedData, bufferInfo);
                }
                mediaCodec.releaseOutputBuffer(encoderStatus, false);
            }
        }
    }

    private static class EncodeHanlder extends Handler{

        public EncodeHanlder(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what){
                case MSG_START:
                    MediaCodec startCodec = (MediaCodec)msg.obj;
                    if(startCodec != null){
                        startCodec.start();
                    }
                    msg.obj = null;
                    break;
                case MSG_STOP:
                    AVCSurfaceEncoder stopCodec = (AVCSurfaceEncoder)msg.obj;
                    if(stopCodec != null){
                        stopCodec.stop();
                    }
                    msg.obj = null;
                    break;
                case MSG_RELEASE:
                    MediaCodec releaseCodec = (MediaCodec)msg.obj;
                    if(releaseCodec != null){
                        releaseCodec.release();
                    }
                    msg.obj = null;
                    Looper.myLooper().quit();
                    break;
                case MSG_FRAME_AVAILABLE:
                    AVCSurfaceEncoder encoder = (AVCSurfaceEncoder)msg.obj;
                    if(encoder != null){
                        encoder.drainEncoder(msg.arg1 > 0);
                    }
                    msg.obj = null;
                    break;
                    default:
            }
        }
    }
}

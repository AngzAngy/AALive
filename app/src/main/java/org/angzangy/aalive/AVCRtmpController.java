package org.angzangy.aalive;

import android.media.MediaCodec;

import org.angzangy.aalive.codec.FrameReceiver;
import org.angzangy.aalive.muxer.RtmpMuxer;
import org.angzangy.aalive.muxer.TimeIndexCounter;

import java.nio.ByteBuffer;

public class AVCRtmpController extends AVCSenderController implements FrameReceiver {
    private String mUrl;
    private RtmpMuxer rtmpMuxer;
    private TimeIndexCounter timeIndexCounter;
    private int index;
    public AVCRtmpController(String url) {
        mUrl = url;
        timeIndexCounter = new TimeIndexCounter();
        setFrameReceiver(this);
        rtmpMuxer = new RtmpMuxer();
    }

    public static class Naul{
        public int startCodePrefixBytes;
        public int bytes;
        public int type;
        public int totalBytes() {
            return bytes + startCodePrefixBytes;
        }
        @Override
        public String toString() {
            StringBuilder builder = new StringBuilder();
            builder.append("[startCodePrefixBytes:").append(startCodePrefixBytes);
            builder.append(",bytes:").append(bytes);
            builder.append(",type:").append(type).append("]");
            return builder.toString();
        }
    }

    private static int getStartCodeBytes(ByteBuffer buffer, int startPos) {
        int pos = startPos;
        if(pos + 2 > buffer.limit()) {
            return 0;
        }
        if(buffer.get(pos++) != 0 || buffer.get(pos++) != 0){
            return 0;
        } else if(buffer.get(pos) == 1) {
            return 3;
        } else if(buffer.get(pos) == 0) {
            pos++;
            if(pos <= buffer.limit() && buffer.get(pos) == 1) {
                return 4;
            }
        }
        return 0;
    }

    private Naul getNaul(ByteBuffer buffer, int startPos) {
        int len = getStartCodeBytes(buffer, startPos);
        Naul naul = null;
        if(len > 0) {
            naul= new Naul();
            naul.startCodePrefixBytes = len;
            len = 0;
            int pos = startPos + naul.startCodePrefixBytes;
            while(pos <= buffer.limit()) {
                len = getStartCodeBytes(buffer, pos);
                if(len > 0) {
                    break;
                }
                pos++;
            }
            if(len <= 0) {
                pos --;
            }
            naul.bytes = pos - startPos - naul.startCodePrefixBytes;
            naul.type = buffer.get(startPos + naul.startCodePrefixBytes) & 0x1f;
        }
        return naul;
    }

    private Naul spsNaul;
    private Naul ppsNaul;
    @Override
    public void receive(ByteBuffer buffer, MediaCodec.BufferInfo bufferInfo) {
        timeIndexCounter.calcTotalTime(bufferInfo.presentationTimeUs);
        buffer.position(bufferInfo.offset);
        buffer.limit(bufferInfo.size);
//        Naul naul = getNaul(buffer, 0);
//        if(naul != null) {
//            switch (naul.type) {
////                case 5:
////                    LogPrinter.e("mybug "+naul+
////                            ",,isKey naul:" + ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_KEY_FRAME) != 0) + ",,totalBytes:"+(naul.totalBytes()+",,realSize:"+bufferInfo.size)+",,index:"+index);
////                    break;
//                case 7:
//                    spsNaul = naul;
//                    if(spsNaul.totalBytes() < bufferInfo.size) {
//                        naul = getNaul(buffer, spsNaul.totalBytes());
//                        if(naul != null && naul.type == 8) {
//                            ppsNaul = naul;
//
//                            LogPrinter.e("javamybug [totalLen:" +bufferInfo.size+"]SPS[type:"+spsNaul.type+",strlen:"+spsNaul.startCodePrefixBytes+",totalLen:"+spsNaul.totalBytes()+"]PPS[type:"+ppsNaul.type+",strlen:"+ppsNaul.startCodePrefixBytes+",totalLen:"+ppsNaul.totalBytes()+"]");
//                        }
//                    }
//                    break;
//                case 8:
//                    break;
//                default:
//            }
//        }
//        index ++;
        if(isConnected()) {
            byte [] buf = new byte[bufferInfo.size];
            buffer.get(buf);
            rtmpMuxer.writeVideo(buf, 0, bufferInfo.size, timeIndexCounter.getTimeIndex());
        }
    }

    @Override
    protected void openStream() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                LogPrinter.d("AVCRtmpController videoSize:"+videoWidth + " x "+videoHeight);
                rtmpMuxer.open(mUrl, videoWidth, videoHeight);
                asyncStartEncoder();
            }
        }, true);
    }

    @Override
    protected void closeStream() {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                if(isConnected()) {
                    rtmpMuxer.close();
                }
            }
        }, true);
    }

    private boolean isConnected() {
        return rtmpMuxer != null && (rtmpMuxer.isConnected());
    }

    @Override
    protected void release() {
        if(isConnected()) {
            rtmpMuxer.close();
            rtmpMuxer.release();
            rtmpMuxer = null;
        }
        asyncReleaseEncoder();
        super.release();
    }
}

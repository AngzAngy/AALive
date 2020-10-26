package org.angzangy.aalive;

import android.media.MediaCodec;

import java.io.File;
import java.nio.ByteBuffer;

public class AVCDumpFileController extends AVCSenderController{
    private FileReceiver fileReceiver;
    public AVCDumpFileController (File file) {
        fileReceiver = new FileReceiver(file){
            ByteBuffer configBuffer;
            @Override
            public void receive(ByteBuffer encodedData, MediaCodec.BufferInfo bufferInfo) {
                encodedData.position(bufferInfo.offset);
                encodedData.limit(bufferInfo.offset + bufferInfo.size);
                if((bufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                    configBuffer = ByteBuffer.allocateDirect(bufferInfo.size);
                    configBuffer.put(encodedData);
                } else {
                    if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_KEY_FRAME) != 0) {
                        // For H.264 key frame prepend SPS and PPS NALs at the start.
                        configBuffer.rewind();
                        write(configBuffer);
                    }
                    write(encodedData);
                }
            }
        };
        setFrameReceiver(fileReceiver);
    }

    @Override
    protected void openStream() {
        if(fileReceiver != null) {
            fileReceiver.openOutputStream();
        }
    }

    @Override
    protected void closeStream() {
        if(fileReceiver != null) {
            fileReceiver.closeOutputStream();
        }
    }
}

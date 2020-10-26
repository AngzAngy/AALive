package org.angzangy.aalive.codec;

import android.media.MediaCodec;

import java.nio.ByteBuffer;

public interface FrameReceiver {
    public void receive(ByteBuffer buffer, MediaCodec.BufferInfo bufferInfo);
}

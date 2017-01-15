package org.angzangy.aalive;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class NioBufferUtil {
    public static ByteBuffer allocByteBuffer(int bblength) {
        ByteBuffer vbb = ByteBuffer.allocateDirect(bblength);
        vbb.order(ByteOrder.nativeOrder());
        vbb.position(0);
        return vbb;
    }
}

package org.angzangy.aalive;

import org.angzangy.aalive.codec.FrameReceiver;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

public abstract class FileReceiver implements FrameReceiver {
    private File file;
    private FileChannel fileChannel;

    public FileReceiver(File dirFile, String fileName) {
        this(new File(dirFile, fileName));
    }

    public FileReceiver(File file) {
        this.file = file;
    }

    public void openOutputStream() {
        try {
            FileOutputStream outputStream = new FileOutputStream(file);
            fileChannel = outputStream.getChannel();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            fileChannel = null;
        }
    }

    public void closeOutputStream() {
        if(fileChannel != null) {
            try {
                fileChannel.close();
            } catch (IOException e) {
                e.printStackTrace();
            }finally {
                fileChannel = null;
            }
        }
    }

    public int write(ByteBuffer buffer) {
        try {
            if (fileChannel != null && fileChannel.isOpen()) {
                return fileChannel.write(buffer);
            }
        }catch (Throwable throwable) {
            throwable.printStackTrace();
        }
        return 0;
    }
}

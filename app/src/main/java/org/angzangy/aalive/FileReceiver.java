package org.angzangy.aalive;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

public class FileReceiver implements DataReceiver {
    private File file;
    private OutputStream outputStream;
    public FileReceiver(File dirFile, String fileName) {
        this(new File(dirFile, fileName));
    }

    public FileReceiver(File file) {
        this.file = file;
    }

    public void openOutputStream() {
        try {
            outputStream = new FileOutputStream(file);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            outputStream = null;
        }
    }

    public void closeOutputStream() {
        if(outputStream != null) {
            try {
                outputStream.close();
            } catch (IOException e) {
                e.printStackTrace();
            }finally {
                outputStream = null;
            }
        }
    }

    @Override
    public void receive(Buffer buffer) {
        if(outputStream != null && buffer!= null && buffer.buf != null) {
            try {
                outputStream.write(buffer.buf, buffer.offsetInBytes, buffer.sizeInBytes);
            } catch (IOException e) {
                e.printStackTrace();
                closeOutputStream();
            }
        }
    }
}

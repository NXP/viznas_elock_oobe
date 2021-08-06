package com.nxp.facemanager.utility.HTTPS;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class IOUtil {
    public static String readFully(InputStream inputStream) throws IOException {

        if(inputStream == null) {
            return "";
        }

        BufferedInputStream bufferedInputStream = null;
        ByteArrayOutputStream byteArrayOutputStream = null;

        try {
            bufferedInputStream = new BufferedInputStream(inputStream);
            byteArrayOutputStream = new ByteArrayOutputStream();

            final byte[] buffer = new byte[1024];
            int available = 0;

            while ((available = bufferedInputStream.read(buffer)) >= 0) {
                byteArrayOutputStream.write(buffer, 0, available);
            }

            return byteArrayOutputStream.toString();

        } finally {
            if(bufferedInputStream != null) {
                bufferedInputStream.close();
            }
        }
    }

    public static byte[] readByteArray(InputStream input) throws IOException
    {
        byte[] buffer = new byte[8192];
        int bytesRead;
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        while ((bytesRead = input.read(buffer)) != -1)
        {
            output.write(buffer, 0, bytesRead);
        }
        return output.toByteArray();
    }
}

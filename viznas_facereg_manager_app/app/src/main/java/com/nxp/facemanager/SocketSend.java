package com.nxp.facemanager;

import android.os.SystemClock;
import android.util.Log;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.Socket;

public class SocketSend {
    private String mPayload = null;
    private String mIp = null;
    private int mPort;
    private final String TAG = Socket.class.getName();

    public Socket mSocket=null;
    private PrintWriter mPrintWriter;

    public SocketSend(String payload, String Ip, int port) {
        mPayload = payload;
        mIp = Ip;
        mPort = port;

        new Thread(new Runnable() {
            @Override
            public void run() {
                //connect to server
                //IO operation must be executed in sub-thread
                connectTCPServer();
            }
        }).start();

    }

    private void connectTCPServer() {
        int loop_count = 0;
        while (mSocket == null) {
            try {
                mSocket = new Socket(mIp, mPort);
            } catch (IOException e) {
                e.printStackTrace();
                Log.e(TAG, "Connection built up failed(" + mIp + ":" + mPort + ")");
            }
            loop_count++;
            if(loop_count > 14)
                return;
            SystemClock.sleep(500);
        }

        //Loop check connection status. If no connection built up, try to create a connection per 1s
        if (mSocket.isConnected()) {
            try {
                //Create Socket object
                mPrintWriter = new PrintWriter(new OutputStreamWriter(mSocket.getOutputStream()),true);
                if (mSocket.isConnected())
                    Log.d(TAG, "Socket connection built up");
            } catch (IOException e) {
                e.printStackTrace();
            }
            Log.d(TAG, "Waiting for Socket built up");
        }

    }

    public int send()
    {
        Log.d(TAG, "Ip:" + mIp + " Port: " + mPort);
        mPrintWriter.println(mPayload);
        mPrintWriter.flush();

        return  0;
    }

    public int close()
    {
        try {
            mSocket.shutdownInput();
            mSocket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return 0;
    }

}

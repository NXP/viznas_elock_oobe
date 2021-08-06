package com.nxp.facemanager.activity;
import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.os.Message;
import android.os.SystemClock;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.nxp.facemanager.R;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import android.os.Handler;
import androidx.annotation.Nullable;
import java.net.Socket;
import androidx.appcompat.app.AppCompatActivity;

public class SocketActivity extends AppCompatActivity implements View.OnClickListener {

    private TextView mTvChatContent;
    private EditText mEtSendContent;
    private Button mButton;
    private String payload;
   // private Intent mIntent;

    private static final int CONNECT_SERVER_SUCCESS = 0;
    private static final int MESSAGE_RECEIVE_SUCCESS = 1;
    private static final int MESSAGE_SEND_SUCCESS=2;
    @SuppressLint("all")
    private Handler mHandler = new Handler(new Handler.Callback() {

        @Override
        public boolean handleMessage(Message msg) {
            switch (msg.what) {
                case CONNECT_SERVER_SUCCESS:
                    mTvChatContent.setText("connection successfull\n");
                    break;
                case MESSAGE_RECEIVE_SUCCESS:
                    String msgContent = mTvChatContent.getText().toString();
                    mTvChatContent.setText(msgContent+msg.obj.toString());
                    break;
                case MESSAGE_SEND_SUCCESS:
                    mEtSendContent.setText("");
                    mTvChatContent.setText(mTvChatContent.getText().toString()+msg.obj.toString()+"\n");
                    break;
            }
            return false;
        }
    });
    private PrintWriter mPrintWriter;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_socket);
        mTvChatContent = findViewById(R.id.tv_chat_content);
        mEtSendContent = findViewById(R.id.et_send_content);
        mButton = findViewById(R.id.but_send);
        mButton.setOnClickListener(this);
        Intent intent = getIntent();
        payload = intent.getStringExtra("payload");
        //start server
       // mIntent = new Intent(this, TCPServerService.class);
        //startService(mIntent);

        new Thread(new Runnable() {
            @Override
            public void run() {
                //connect to server
                //IO operation must be executed in sub-thread
                connectTCPServer();
            }
        }).start();

    }

    private  Socket mSocket=null;
    private void connectTCPServer() {
        //Loop check connection status. If no connection built up, try to create a connection per 1s
        while (mSocket == null) {
            try {
                //Create Socket object
                mSocket = new Socket("192.168.1.102", 7);
                mPrintWriter = new PrintWriter(new OutputStreamWriter(mSocket.getOutputStream()),true);
                if (mSocket.isConnected())
                    mHandler.sendEmptyMessage(CONNECT_SERVER_SUCCESS);
            } catch (IOException e) {
                e.printStackTrace();
                //sleep. 1s later, try again.
                SystemClock.sleep(1000);
            }
        }
        //Receive message in loop
        try {
            BufferedReader reader = new BufferedReader(new InputStreamReader(mSocket.getInputStream()));
            while (!SocketActivity.this.isFinishing()){
                int aa = reader.read();
               // if (!TextUtils.isEmpty(aa)){
                if (aa != -1) {
                    //Notify UI
                    mHandler.obtainMessage(MESSAGE_RECEIVE_SUCCESS, ((char) aa)).sendToTarget();
                    aa = -1;
                }
            }
            //Close stream
            mPrintWriter.close();
            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @SuppressLint("SetTextI18n")
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.but_send:
                new Thread(new Runnable() {
                    @Override
                    public void run() {
//                        String msg = mEtSendContent.getText().toString();
//                        if (mPrintWriter!=null && !TextUtils.isEmpty(msg)){
                            Log.i(TAG, "payload = "+payload);
                            mPrintWriter.println(payload);
                            //This operation is not necessary, since stream is flushed in constructor method.
                            mPrintWriter.flush();
                            //Notify UI
                            mHandler.obtainMessage(MESSAGE_SEND_SUCCESS,payload).sendToTarget();
                        }
//                    }
                }).start();


                break;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        //Close input stream and connection
        if (mSocket!=null){
            try {
                mSocket.shutdownInput();
                mSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        //Stop service
        //stopService(mIntent);
    }
    private static final String TAG = "TCPServerService";
}

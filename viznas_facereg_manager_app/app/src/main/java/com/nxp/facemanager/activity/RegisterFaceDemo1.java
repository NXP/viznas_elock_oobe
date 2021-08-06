package com.nxp.facemanager.activity;

import android.os.Bundle;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import android.os.SystemClock;
import android.preference.EditTextPreference;
import android.text.Editable;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.nxp.facemanager.R;
import com.nxp.facemanager.SocketSend;


public class RegisterFaceDemo1 extends AppCompatActivity {
    private Button mButton;
    private EditText mEditIP;
    private EditText mEditPort;
    private TextView mTextView;
    private SocketSend mObj;
    private String mIp = " ";
    private int mPort = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_register_face_demo);
        mButton = findViewById(R.id.button_demo);
        mEditIP = findViewById(R.id.Edit_IP);
        mEditPort = findViewById(R.id.Edit_Port);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

    }

    public void onClick(View view) {
        String IP = mEditIP.getText().toString();
        int port = Integer.parseInt(mEditPort.getText().toString());
        if (!(IP.equals(mIp)) || mPort != port) {
            mIp = IP;
            mPort = port;
            if(mObj != null) {
                mObj.close();
            }
            mObj = new SocketSend("Hello device board", mIp, mPort);
        }
        while(mObj.mSocket == null || !mObj.mSocket.isConnected()) {
            SystemClock.sleep(1000);
        }
        mObj.send();
    }
}

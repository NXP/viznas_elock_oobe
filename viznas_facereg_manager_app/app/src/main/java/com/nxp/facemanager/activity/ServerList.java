package com.nxp.facemanager.activity;



import android.util.Log;
import android.widget.EditText;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.nxp.facemanager.R;

import java.util.ArrayList;
import java.util.List;



import android.os.Bundle;
import android.provider.Settings;

import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;


import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.databinding.DataBindingUtil;
import androidx.lifecycle.ViewModelProviders;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.ItemTouchHelper;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

class ServerInfo {
    String mserverName;
    String mServerIp;
    String mconnect;

    ServerInfo(String name, String connect, String serverIp) {
        this.mserverName = name;
        this.mServerIp = serverIp;
        this.mconnect = connect;
    }
}

public class ServerList extends AppCompatActivity  {
    private RecyclerView mRV;
    private RVAdapter mAdapter;
    private List<ServerInfo> mServer;
    private EditText mEditText;
    private int mConnectStatus = 1;

    // This method creates an ArrayList that has three Person objects
// Checkout the project associated with this tutorial on Github if
// you want to use the same images.

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_server_list);
        Toolbar toolbar = findViewById(R.id.toolbar_server);

        setSupportActionBar(toolbar);
       // FloatingActionButton fab = findViewById(R.id.fab_server);
        /*fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Log.e("fab", "fab on click");

                int serverListSize = mServer.size();
                // Add a new word to the wordList.
                Log.e("fab", "add element");
                mServer.add(new ServerInfo("Server" + serverListSize, "Connect", "192.168.8.8"));
                // Notify the adapter, that the data has changed.
                Log.e("fab", "notify inserting operation");
                mRV.getAdapter().notifyItemInserted(serverListSize);
                // Scroll to the bottom.
                Log.e("fab", "smoothScrollToPosition");
                mRV.smoothScrollToPosition(serverListSize);
            }
        });*/

        Log.e("MainActivity", "Instance mRecyclerView");

        // Create an adapter and supply the data to be displayed.
        Log.e("MainActivity", "WordListAdapter");
        mEditText = findViewById(R.id.textView);
        mServer = new ArrayList<>();
        mServer.add(new ServerInfo("Server1", "Connect", mEditText.getText().toString()));
        mRV = (RecyclerView)findViewById(R.id.recyclerview);
        mRV.setLayoutManager(new LinearLayoutManager(this));
        mAdapter = new RVAdapter(mServer);
        mRV.setAdapter(mAdapter);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.activity_server_list_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public void connect_handler(View view) {
        TextView V = (TextView) view;
        if (mConnectStatus == 1) {
            mConnectStatus = 2;
            V.setText("Connected");
        } else if (mConnectStatus == 2) {
            mConnectStatus = 1;
            V.setText("Connect");
        }
    }
}

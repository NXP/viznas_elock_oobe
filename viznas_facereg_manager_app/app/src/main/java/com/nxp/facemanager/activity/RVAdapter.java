package com.nxp.facemanager.activity;

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import com.nxp.facemanager.R;

import java.util.List;

import androidx.cardview.widget.CardView;
import androidx.recyclerview.widget.RecyclerView;

//import androidx.cardview.widget.CardView;
//import androidx.recyclerview.widget.RecyclerView;

//import androidx.cardview.widget.CardView;
//import androidx.recyclerview.widget.RecyclerView;


public class RVAdapter extends
        RecyclerView.Adapter<RVAdapter.ServerViewHolder>  {
    private List<ServerInfo> mServer;

    private LayoutInflater mInflater;

    RVAdapter(List<ServerInfo> server){
        this.mServer = server;
    }

    public class ServerViewHolder extends RecyclerView.ViewHolder {
        CardView cv;
        TextView serverName;
        TextView serverIp;
        TextView connect;

        ServerViewHolder(View itemView) {
            super(itemView);
            cv = (CardView)itemView.findViewById(R.id.cv);
            serverName = (TextView)itemView.findViewById(R.id.server_name);
            serverIp = (TextView)itemView.findViewById(R.id.server_ip);
            connect = (TextView)itemView.findViewById(R.id.Connect);
        }
    }


    @Override
    public ServerViewHolder onCreateViewHolder( ViewGroup viewGroup, int i) {
        Log.e("RVAdapter", "onCreateViewHolder");
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.server_listitems, viewGroup, false);
        ServerViewHolder pvh = new ServerViewHolder(v);
        return pvh;
    }

    @Override
    public void onBindViewHolder( ServerViewHolder personViewHolder, int i) {
        personViewHolder.serverName.setText(mServer.get(i).mserverName);
        personViewHolder.serverIp.setText(mServer.get(i).mServerIp);
        personViewHolder.connect.setText(mServer.get(i).mconnect);
    }


    @Override
    public int getItemCount() {
        Log.e("server_listitem", "getItemCount = " + mServer.size());
        return mServer.size();
    }

    @Override
    public void onAttachedToRecyclerView(RecyclerView recyclerView) {
        super.onAttachedToRecyclerView(recyclerView);
    }
}

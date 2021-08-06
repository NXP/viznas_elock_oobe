package com.nxp.facemanager.fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebViewClient;

import com.nxp.facemanager.R;
import com.nxp.facemanager.databinding.FragmentHelpBinding;

import androidx.annotation.NonNull;
import androidx.databinding.DataBindingUtil;
import androidx.fragment.app.Fragment;

/**
 * A simple {@link Fragment} subclass.
 * Activities that contain this fragment must implement the
 * to handle interaction events.
 * Use the {@link HelpFragment#newInstance} factory method to
 * create an instance of this fragment.
 */
public class HelpFragment extends BaseFragment {


    public HelpFragment() {
        // Required empty public constructor
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @return A new instance of fragment SmartLocksFragment.
     */
    public static HelpFragment newInstance() {
        return new HelpFragment();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        FragmentHelpBinding binding = DataBindingUtil.inflate(
                inflater, R.layout.fragment_help, container, false);
        binding.webview.setWebViewClient(new WebViewClient());
        binding.webview.loadUrl("https://www.nxp.com/");
        return binding.getRoot();
    }


}

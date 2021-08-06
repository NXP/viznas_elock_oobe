package com.nxp.facemanager.activity;

import android.view.View;

/**
 * Listener for recycler view Item click event
 */
public interface OnItemClickListener {
    /**
     * This item click {@link Override}
     *
     * @param item   object of item type
     * @param view view id.
     */
    void onItemClick(Object item, View view);
}

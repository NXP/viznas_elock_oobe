package com.nxp.facemanager.activity;

import android.view.View;

/**
 * Listener for recycler view Item long click event
 */
public interface OnItemLongClickListener {
    /**
     * This item click {@link Override}
     *
     * @param item object of item type
     * @param view view id.
     */
    void onItemLongClick(Object item, View view);
}

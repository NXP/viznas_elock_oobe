package com.nxp.facemanager.model;

/**
 * Used with {@link org.greenrobot.eventbus.EventBus } to hide/show the back button on toolbar.
 */
public class ChangeHomeIcon {
    /**
     * Boolean to detect the value.
     * False = Hamburger Visible.
     * True = Hamburger Invisible.
     */
    private boolean isBackEnable;

    public ChangeHomeIcon(boolean isBackEnable) {
        this.isBackEnable = isBackEnable;
    }

    public boolean isBackEnable() {
        return isBackEnable;
    }

    public void setBackEnable(boolean backEnable) {
        isBackEnable = backEnable;
    }
}

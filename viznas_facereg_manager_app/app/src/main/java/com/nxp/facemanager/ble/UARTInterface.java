package com.nxp.facemanager.ble;

/**
 * The interface that will be used between UART service and UART Manager
 * for sending bytes data from activity/fragment using Uart service instance
 */
public interface UARTInterface {

    void sendBytes(byte[] data);
}

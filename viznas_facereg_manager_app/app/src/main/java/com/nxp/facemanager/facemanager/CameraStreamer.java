/* Copyright 2013 Foxdog Studios Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.nxp.facemanager.facerec;

import android.app.Activity;
import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.SystemClock;
import android.view.Surface;
import android.view.TextureView;

import com.nxp.facemanager.utility.AppLogger;

import java.util.List;


@SuppressWarnings({"FieldCanBeLocal", "deprecation"})
public class CameraStreamer {

    private static final String TAG = CameraStreamer.class.getSimpleName();

    private static final int MESSAGE_TRY_START_STREAMING = 0;
    private static final int MESSAGE_SEND_PREVIEW_FRAME = 1;

    private static final long OPEN_CAMERA_POLL_INTERVAL_MS = 1000L;

    private final Object mLock = new Object();
    private final com.nxp.facemanager.facerec.MovingAverage mAverageSpf = new com.nxp.facemanager.facerec.MovingAverage(90 /* numValues */);

    private final int mPreviewSizeIndex;
    private final int mJpegQuality;
    private final TextureView mPreviewDisplay;
    private final Context context;
    private boolean mRunning = false;
    private Looper mLooper = null;
    private Handler mWorkHandler = null;
    // onPreviewFrame(byte[], Camera)
    private final Camera.PreviewCallback mPreviewCallback = (data, camera) -> {
        final Long timestamp = SystemClock.elapsedRealtime();
        final Message message = mWorkHandler.obtainMessage();
        message.what = MESSAGE_SEND_PREVIEW_FRAME;
        message.obj = new Object[]{data, camera, timestamp};
        message.sendToTarget();
    }; // mPreviewCallback
    private Camera mCamera = null;
    private int mPreviewFormat = Integer.MIN_VALUE;
    private int mPreviewWidth = 640;
    private int mPreviewHeight = 480;
    private Rect mPreviewRect = null;
    private int mPreviewBufferSize = Integer.MIN_VALUE;
    private com.nxp.facemanager.facerec.MemoryOutputStream mJpegOutputStream = null;
    private com.nxp.facemanager.facerec.MJpegHttpStreamer mMJpegHttpStreamer = null;
    private long mNumFrames = 0L;
    private long mLastTimestamp = Long.MIN_VALUE;

    /* package */
    public CameraStreamer(Context context, final boolean useFlashLight,
                          final int previewSizeIndex, final int jpegQuality, final TextureView previewDisplay) {
        super();
        this.context = context;
        if (previewDisplay == null) {
            throw new IllegalArgumentException("previewDisplay must not be null");
        } // if

        mPreviewSizeIndex = previewSizeIndex;
        mJpegQuality = jpegQuality;
        mPreviewDisplay = previewDisplay;

    } // constructor(SurfaceHolder)


    /* package */
    public void start() {
        synchronized (mLock) {
            if (mRunning) {
                throw new IllegalStateException("CameraStreamer is already running");
            } // if
            mRunning = true;
        } // synchronized

        final HandlerThread worker = new HandlerThread(TAG, Process.THREAD_PRIORITY_MORE_FAVORABLE);
        worker.setDaemon(true);
        worker.start();
        mLooper = worker.getLooper();
        mWorkHandler = new WorkHandler(mLooper);
        mWorkHandler.obtainMessage(MESSAGE_TRY_START_STREAMING).sendToTarget();
    } // start()

    /**
     * Stop the image streamer. The camera will be released during the
     * execution of stop() or shortly after it returns. stop() should
     * be called on the main thread.
     */
    /* package */
    public void stop() {
        synchronized (mLock) {
            if (!mRunning) {
                throw new IllegalStateException("CameraStreamer is already stopped");
            } // if

            mRunning = false;
            if (mMJpegHttpStreamer != null) {
                mMJpegHttpStreamer.stop();
            } // if
            if (mCamera != null) {
                mCamera.release();
                mCamera = null;
            } // if
        } // synchronized
        mLooper.quit();
    } // stop()

    private void tryStartStreaming() {
        try {
            while (true) {
                try {
                    startStreamingIfRunning();
                } //try
                catch (final RuntimeException openCameraFailed) {
                    AppLogger.d("Open camera failed, retying in " + OPEN_CAMERA_POLL_INTERVAL_MS
                            + "ms", openCameraFailed);
                    Thread.sleep(OPEN_CAMERA_POLL_INTERVAL_MS);
                    continue;
                } // catch
                break;
            } // while
        } // try
        catch (final Exception startPreviewFailed) {
            // Captures the IOException from startStreamingIfRunning and
            // the InterruptException from Thread.sleep.
            AppLogger.w(TAG, "Failed to start camera preview", startPreviewFailed);
        } // catch
    } // tryStartStreaming()

    private void startStreamingIfRunning() {
        // Throws RuntimeException if the camera is currently opened
        // by another application.

        Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
        int cameraCount;
        cameraCount = Camera.getNumberOfCameras();
        Camera camera = null;

        for (int camIdx = 0; camIdx < cameraCount; camIdx++) {
            Camera.getCameraInfo(camIdx, cameraInfo);
            if (cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                try {
                    camera = Camera.open(camIdx);
                } catch (RuntimeException e) {
                    AppLogger.e(TAG, "Camera failed to open: " + e.getLocalizedMessage());
                }
            }
        }

        if (camera == null) {
            AppLogger.e(TAG, "Front camera not found!!!");
            return;
        }


        final Camera.Parameters params = camera.getParameters();
        camera.setDisplayOrientation(getCorrectCameraOrientation(cameraInfo, mCamera));

        final List<Camera.Size> supportedPreviewSizes = params.getSupportedPreviewSizes();
        params.setPreviewSize(640, 480);

        // params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);


        camera.setParameters(params);

        // Set up preview callback
        mPreviewFormat = params.getPreviewFormat();
        final Camera.Size previewSize = params.getPreviewSize();
        mPreviewWidth = previewSize.width;
        mPreviewHeight = previewSize.height;
        final int BITS_PER_BYTE = 8;
        final int bytesPerPixel = ImageFormat.getBitsPerPixel(mPreviewFormat) / BITS_PER_BYTE;
        // XXX: According to the documentation the buffer size can be
        // calculated by width * height * bytesPerPixel. However, this
        // returned an error saying it was too small. It always needed
        // to be exactly 1.5 times larger.
        mPreviewBufferSize = mPreviewWidth * mPreviewHeight * bytesPerPixel * 3 / 2 + 1;
        AppLogger.e(TAG, "Preview buffer size:  " + mPreviewBufferSize);

        camera.addCallbackBuffer(new byte[mPreviewBufferSize]);
        mPreviewRect = new Rect(0, 0, mPreviewWidth, mPreviewHeight);
        camera.setPreviewCallbackWithBuffer(mPreviewCallback);

        // We assumed that the compressed image will be no bigger than
        // the uncompressed image.
        mJpegOutputStream = new com.nxp.facemanager.facerec.MemoryOutputStream(mPreviewBufferSize);

        final com.nxp.facemanager.facerec.MJpegHttpStreamer streamer = new com.nxp.facemanager.facerec.MJpegHttpStreamer(mPreviewBufferSize);
        streamer.start();

        synchronized (mLock) {
//            if (!mRunning) {
//                streamer.stop();
//                camera.release();
//                return;
//            } // if
//
//            try {
//                camera.setPreviewDisplay(mPreviewDisplay);
//            } // try
//            catch (final IOException e) {
//                streamer.stop();
//                camera.release();
//                throw e;
//            } // catch

            mMJpegHttpStreamer = streamer;
            camera.startPreview();
            mCamera = camera;
        } // synchronized
    } // startStreamingIfRunning()

    private void sendPreviewFrame(byte[] data, final Camera camera, final long timestamp) {

        // Calculate the timestamp
        final long MILLI_PER_SECOND = 1000L;
        final long timestampSeconds = timestamp / MILLI_PER_SECOND;

        // Update and log the frame rate
        final long LOGS_PER_FRAME = 10L;
        mNumFrames++;

        if (mLastTimestamp != Long.MIN_VALUE) {
            mAverageSpf.update(timestampSeconds - mLastTimestamp);
            if (mNumFrames % LOGS_PER_FRAME == LOGS_PER_FRAME - 1) {
                AppLogger.d("FPS: " + 1.0 / mAverageSpf.getAverage());
            } // if
        } // else

        mLastTimestamp = timestampSeconds;

        //AppLogger.e(TAG, "previewWidth-> " + mPreviewWidth + " mPreviewHeight-> "  + mPreviewHeight);

        // Create JPEG
        YuvImage image = new YuvImage(data, mPreviewFormat, mPreviewWidth, mPreviewHeight,
                null /* strides */);

        image.compressToJpeg(mPreviewRect, 100, mJpegOutputStream);

        mMJpegHttpStreamer.streamJpeg(mJpegOutputStream.getBuffer(), mJpegOutputStream.getLength(),
                timestamp);

        // Clean up
        mJpegOutputStream.seek(0);
        // XXX: I believe that this is thread-safe because we're not
        // calling methods in other threads. I might be wrong, the
        // documentation is not clear.
        camera.addCallbackBuffer(data);

       /*if(sendFrame == SEND_FRAME_MAX) {
           RotateTask rotateTask = new RotateTask(camera, data, timestamp);
           rotateTask.execute(image);
           sendFrame = 0;
       } else {
           // Clean up
           mJpegOutputStream.seek(0);
           // XXX: I believe that this is thread-safe because we're not
           // calling methods in other threads. I might be wrong, the
           // documentation is not clear.
           camera.addCallbackBuffer(data);
       }*/

       /*RotateTask rotateTask = new RotateTask(camera, data, timestamp);
       rotateTask.execute(image);*/


    } // sendPreviewFrame(byte[], camera, long)


    private int getCorrectCameraOrientation(Camera.CameraInfo info, Camera camera) {

        int rotation = ((Activity) context).getWindowManager().getDefaultDisplay().getRotation();
        int degrees = 0;

        switch (rotation) {
            case Surface.ROTATION_0:
                degrees = 0;
                break;

            case Surface.ROTATION_90:
                degrees = 90;
                break;

            case Surface.ROTATION_180:
                degrees = 180;
                break;

            case Surface.ROTATION_270:
                degrees = 270;
                break;

        }

        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360;
        } else {
            result = (info.orientation - degrees + 360) % 360;
        }

        return result;
    }

    private String capitalize(String s) {
        if (s == null || s.length() == 0) {
            return "";
        }
        char first = s.charAt(0);
        if (Character.isUpperCase(first)) {
            return s;
        } else {
            return Character.toUpperCase(first) + s.substring(1);
        }
    }

    private final class WorkHandler extends Handler {
        private WorkHandler(final Looper looper) {
            super(looper);
        } // constructor(Looper)

        @Override
        public void handleMessage(final Message message) {
            switch (message.what) {
                case MESSAGE_TRY_START_STREAMING:
                    tryStartStreaming();
                    break;
                case MESSAGE_SEND_PREVIEW_FRAME:
                    final Object[] args = (Object[]) message.obj;
                    sendPreviewFrame((byte[]) args[0], (Camera) args[1], (Long) args[2]);
                    break;
                default:
                    throw new IllegalArgumentException("cannot handle message");
            } // switch
        } // handleMessage(Message)
    } // class WorkHandler


}




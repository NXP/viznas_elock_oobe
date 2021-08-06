package com.nxp.facemanager.activity;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.graphics.drawable.Drawable;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.provider.Settings;
import android.util.Base64;
import android.util.Log;
import android.util.Size;
import android.util.SparseIntArray;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.RadioButton;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.nxp.facemanager.Oasis;
import com.nxp.facemanager.R;
import com.nxp.facemanager.database.entity.UserInformation;
import com.nxp.facemanager.databinding.ActivityTrainingBinding;
import com.nxp.facemanager.utility.AppLogger;
import com.nxp.facemanager.utility.AutoFitTextureView;
import com.nxp.facemanager.viewModels.TrainingModel;
import com.nxp.facemanager.utility.AppConstants;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import androidx.annotation.NonNull;
import androidx.appcompat.widget.Toolbar;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.databinding.DataBindingUtil;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModelProviders;

import static com.nxp.facemanager.fragment.SmartUsersFragment.IS_SKIPPABLE;
import static com.nxp.facemanager.viewModels.CreateUserViewModel.IS_EDIT_USER;

//@SuppressWarnings("SuspiciousNameCombination")
public class TrainingActivity extends BaseActivity {
    public static final String USERNAME = "username";
    public static final String USER_FEATURES = "user_features";
    public static final String USER_ID = "user_id";

    public int OASIS_W = 1280;
    public int OASIS_H = 1280;


    private TextView mFaceName;
    private ImageView mFaceView;
    private ImageView mFaceBox;
    private Bitmap mFaceImg = null;
    private Button mFaceAdd;
    private Toolbar mToolBar;
    private String mName = "";
    private byte[] mFeature;
    private int[] mBox;
    private Oasis mOasis;
    private boolean DONE_CAPTURE;
    private int[] mId;

    private List<UserInformation> userInformationList;

    static void Remote_convertHexString2Bytes(byte[] ascii, int len, byte[] features)
    {
        byte[] nibble = new byte[2];

        for (int i = 0; i < len; i++) {
            for (int j = 0; j < 2; j++) {
                if (ascii[2*i + j] <= 'f' && ascii[2*i + j] >= 'a') {
                    nibble[j] = (byte) (ascii[2*i + j] - 'a' + 10);
                } else if (ascii[2*i + j] <= '9' && ascii[2*i + j] >= '0') {
                    nibble[j] = (byte) (ascii[2*i + j] - '0');
                } else {
                    nibble[j] = 0;
                }
            }

            features[i] = (byte) (nibble[0] << 4 | nibble[1]);
        }
    }

    private byte[] getPixelsRGBA(Bitmap image) {
        // calculate how many bytes our image consists of
        int bytes = image.getByteCount();
        ByteBuffer buffer = ByteBuffer.allocate(bytes); // Create a new buffer
        image.copyPixelsToBuffer(buffer); // Move the byte data to the buffer
        byte[] temp = buffer.array(); // Get the underlying array containing the
        int rgbBytes = bytes / 4 * 3;
        // Log.e(TAG, "bytes" + bytes + "rgb" + rgbBytes);
        byte[] RGB = new byte[rgbBytes];
        for (int i = 0; i < bytes / 4; i++) {
            RGB[i * 3] = temp[i * 4];
            RGB[i * 3 + 1] = temp[i * 4 + 1];
            RGB[i * 3 + 2] = temp[i * 4 + 2];
            /*
             * if (i < 16) { Log.e(TAG, "[" + byteTo16(temp[i*4]) + "," +
             * byteTo16(temp[i*4+1]) + "," + byteTo16(temp[i*4+2]) + "," +
             * byteTo16(temp[i*4+3])+ "]"); }
             */
        }
        return RGB;
    }

    private Bitmap rgbToGrayscale(Bitmap bm){
        int w = bm.getWidth();
        int h = bm.getHeight();
        Bitmap gray = Bitmap.createBitmap(w, h, bm.getConfig());
        Canvas canvas = new Canvas(gray);
        Paint paint = new Paint();
        ColorMatrix cm = new ColorMatrix();
        cm.setSaturation(0);
        ColorMatrixColorFilter filter = new ColorMatrixColorFilter(cm);
        paint.setColorFilter(filter);
        canvas.drawBitmap(bm, 0,0, paint);

        return gray;
    }
    /**
     * Store the current orientation.
     */
    private static final SparseIntArray ORIENTATIONS = new SparseIntArray();
    /**
     * Camera permission request callback reference.
     */
    private static final int REQUEST_CAMERA_PERMISSION = 1;
    public static final String BASE_64_IMAGE = "base_image";

    /**
     * Menu item from camera {@link menu}
     */
    MenuItem menuCamera;

    private static final String FRAGMENT_DIALOG = "dialog";
    /**
     * Tag for the {@link Log}.
     */
    private static final String TAG = "zdsTA";
    /**
     * Camera state: Showing camera preview.
     */
    private static final int STATE_PREVIEW = 0;
    /**
     * Camera state: Waiting for the focus to be locked.
     */
    private static final int STATE_WAITING_LOCK = 1;
    /**
     * Camera state: Waiting for the exposure to be precapture state.
     */
    private static final int STATE_WAITING_PRECAPTURE = 2;
    /**
     * Camera state: Waiting for the exposure state to be something other than
     * precapture.
     */
    private static final int STATE_WAITING_NON_PRECAPTURE = 3;
    /**
     * Camera state: Picture was taken.
     */
    private static final int STATE_PICTURE_TAKEN = 4;
    private static final int STATE_FACE_DETECT = 5;
    private static final int STATE_DETECTION_DONE = 6;

    /**
     * Max preview width that is guaranteed by Camera2 API
     */
    private static final int MAX_PREVIEW_WIDTH = 1920;
    /**
     * Max preview height that is guaranteed by Camera2 API
     */
    private static final int MAX_PREVIEW_HEIGHT = 1080;

    String mConnectionType;
    String mSmartLockIP;

    /**
     * ID of the current {@link CameraDevice}.
     */
    private String mCameraId;

    static {
        ORIENTATIONS.append(Surface.ROTATION_0, 90);
        ORIENTATIONS.append(Surface.ROTATION_90, 0);
        ORIENTATIONS.append(Surface.ROTATION_180, 270);
        ORIENTATIONS.append(Surface.ROTATION_270, 180);
    }

    /**
     * Data binding object reference of current activity which bind with {@link xml
     * }
     */
    private ActivityTrainingBinding activityTrainingBinding;
    /**
     * The {@link Size} of camera preview.
     */
    private Size mPreviewSize;
    /**
     * An {@link AutoFitTextureView} for camera preview.
     */
    private AutoFitTextureView mTextureView;
    /**
     * A {@link CameraCaptureSession } for camera preview.
     */
    private CameraCaptureSession mCaptureSession;
    /**
     * A reference to the opened {@link CameraDevice}.
     */
    private CameraDevice mCameraDevice;
    /**
     * A {@link CameraCaptureSession.CaptureCallback} that handles events related to
     * JPEG capture.
     */
    private CameraCaptureSession.CaptureCallback mCaptureCallback = new CameraCaptureSession.CaptureCallback() {

        private void process(CaptureResult result) {
            switch (mState) {
                case STATE_PREVIEW: {
                    // We have nothing to do when the camera preview is working normally.
//                    activityTrainingBinding.btnCamera.setVisibility(View.VISIBLE);
                    break;
                }
                case STATE_FACE_DETECT: {
//                    activityTrainingBinding.btnCamera.setVisibility(View.INVISIBLE);
                    getFace();
                    break;
                }
                case STATE_DETECTION_DONE: {

                    break;
                }
                case STATE_WAITING_LOCK: {
                    Integer afState = result.get(CaptureResult.CONTROL_AF_STATE);
                    if (afState == null) {
                        captureStillPicture();
                    } else if (CaptureResult.CONTROL_AF_STATE_FOCUSED_LOCKED == afState
                            || CaptureResult.CONTROL_AF_STATE_NOT_FOCUSED_LOCKED == afState) {
                        // CONTROL_AE_STATE can be null on some devices
                        Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                        if (aeState == null || aeState == CaptureResult.CONTROL_AE_STATE_CONVERGED) {
                            mState = STATE_PICTURE_TAKEN;
                            captureStillPicture();
                        } else {
                            runPrecaptureSequence();
                        }
                    } else {
                        mState = STATE_PICTURE_TAKEN;
                        captureStillPicture();
                    }
                    break;
                }
                case STATE_WAITING_PRECAPTURE: {
                    // CONTROL_AE_STATE can be null on some devices
                    Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                    if (aeState == null || aeState == CaptureResult.CONTROL_AE_STATE_PRECAPTURE
                            || aeState == CaptureRequest.CONTROL_AE_STATE_FLASH_REQUIRED) {
                        mState = STATE_WAITING_NON_PRECAPTURE;
                    }
                    break;
                }
                case STATE_WAITING_NON_PRECAPTURE: {
                    // CONTROL_AE_STATE can be null on some devices
                    Integer aeState = result.get(CaptureResult.CONTROL_AE_STATE);
                    if (aeState == null || aeState != CaptureResult.CONTROL_AE_STATE_PRECAPTURE) {
                        mState = STATE_PICTURE_TAKEN;
                        captureStillPicture();
                    }
                    break;
                }
            }
        }

        private void getFace() {

            Bitmap bm = mTextureView.getBitmap();
            int width = bm.getWidth();
            int height = bm.getHeight();
            byte[] imageData;
            Log.d(TAG, "original image w & h:" + width + " " + height);

            if (width>OASIS_W || height>OASIS_H){
                float scaleWidth = ((float) OASIS_W)/width;
                float scaleHeight = ((float) OASIS_H)/height;
                float scale = Math.min(scaleHeight, scaleWidth);
                Matrix matrix = new Matrix();
                matrix.setScale(scale, scale);
                Bitmap newBm = Bitmap.createBitmap(bm, 0,0,width, height, matrix, false);
                bm = Bitmap.createBitmap(bm, 0,0,width, height, matrix, false);
//                imageData = getPixelsRGBA(newBm);

                width  = bm.getWidth();
                height = bm.getHeight();
                Log.e(TAG, "scaled image w & h:" + width + " " + height);
//                bm.recycle();
//                bm = newBm;

            }
            final Bitmap final_bm = bm;
            if (isElock == 1) {
                final Bitmap grayBitmap = rgbToGrayscale(bm);
                imageData = getPixelsRGBA(grayBitmap);
                Log.e(TAG, "eLock: GREY888");
            } else {
                imageData = getPixelsRGBA(bm);
                Log.e(TAG, "Door access: RGB888");
            }

            mBox = new int[4];
            mBox[0] = -1;
            mBox[1] = -1;
            mBox[2] = -1;
            mBox[3] = -1;

//            mName = mOasis.faceRecognize(imageData, width, height, mFeature, mBox);

            int isEditUser = getIntent().getExtras().getBoolean(AppConstants.IS_EDIT_USER)? 1:0;
            mId = new int[1];
            mFeature = new byte[mOasis.getFaceItemSize()];

            int[] id_db = new int[userInformationList.size()];
            byte[][] feature_db = new byte[userInformationList.size()][mOasis.getFaceItemSize()];
//            int i = 0;
//            for(UserInformation information : userInformationList) {
//                id_db[i] = Integer.parseInt(information.get_id());
//                byte[] feature_tmp = information.getTrainingData().getBytes();
//                Remote_convertHexString2Bytes(feature_tmp, mOasis.getFaceItemSize(), feature_db[i]);
//                i++;
//            };

            int[] result = {-1};

            mName = mOasis.faceRecognize(imageData, width, height, mBox, isEditUser, mId, mFeature, id_db, feature_db, result);
            Log.e(TAG, mName + "[" + mBox[0] + "," + mBox[1] + "," + mBox[2] + "," + mBox[3] + "]");

             TrainingActivity.this.runOnUiThread(new Runnable() {
                 @Override
                 public void run() {
                     try {
                         if (result[0] == 0){
                             activityTrainingBinding.txGuide.setText("Capturing...");
                             return;
                         } else if (result[0] == 1) {
                             activityTrainingBinding.txGuide.setText("User already exists...");
                         } else {
                             activityTrainingBinding.txGuide.setText("Face quality not OK...");
                         }
                     } catch (NullPointerException e) {
                         e.printStackTrace();
                     }
                 }
             });

             Bitmap drawBitmap = Bitmap.createBitmap(bm.getWidth(), bm.getHeight(), Bitmap.Config.ARGB_8888);

             Canvas canvas = new Canvas(drawBitmap);
             Paint paint = new Paint();
            paint.setColor(Color.RED);
             paint.setStyle(Paint.Style.STROKE);
             paint.setStrokeWidth(5);
            if(mBox[0]!=-1){
                canvas.drawRect(mBox[0], mBox[1], mBox[2], mBox[3], paint);
            }
            TrainingActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    try {
                        mFaceBox.setImageBitmap(drawBitmap);
//                    Log.i(TAG, "box drawn");
                    } catch (NullPointerException e) {
                        e.printStackTrace();
                    }
                }
            });
//
//             TrainingActivity.this.runOnUiThread(new Runnable() {
//                 @Override
//                 public void run() {
//                     try {
//                         mFaceView.setImageBitmap(drawBitmap);
//                         Log.i(TAG, "box drawn");
//                     } catch (NullPointerException e) {
//                         e.printStackTrace();
//                     }
//                 }
//             });

            // if a face is detected
            if (mBox[0] != -1) {
                mState = STATE_DETECTION_DONE;
//                new Thread(() -> {

                        try {
                            Bitmap bitmap = Bitmap.createBitmap(final_bm, mBox[0], mBox[1], mBox[2]-mBox[0], mBox[3]-mBox[1]);
                            Bitmap resized = Bitmap.createScaledBitmap(bitmap, 100, 100, true);
                            Matrix matrix = new Matrix();
//                                        matrix.postRotate(-90);
                            matrix.postRotate(0);
                            Bitmap rotatedBitmap = Bitmap.createBitmap(resized, 0, 0, resized.getWidth(), resized.getHeight(), matrix, true);
                            //need to check with rajesh when to enable below line
                            // activityTrainingBinding.getVm().getUserPhoto().set(value);
                            ByteArrayOutputStream out = new ByteArrayOutputStream();
                            rotatedBitmap.compress(Bitmap.CompressFormat.PNG, 50, out);
//                            Log.e("feature data: %b", mFeature);
                            Intent intent = new Intent();
                            if (null != bitmap) {
                                String strBase64Bitmap = Base64.encodeToString(out.toByteArray(), Base64.DEFAULT);
                                intent.putExtra(BASE_64_IMAGE, strBase64Bitmap);
                                //intent.putExtra(USERNAME, mName);
                                intent.putExtra(USER_ID, mId[0]);

                                intent.putExtra(USER_FEATURES, mFeature);
                            }
                            setResult(Activity.RESULT_OK, intent);
                            mOasis.Exit();
                            TrainingActivity.this.finish();

//                            if (!isSkippable) {
////                                Intent resultIntent = new Intent();
//                                String strBase64Bitmap = Base64.encodeToString(out.toByteArray(), Base64.DEFAULT);
//                                intent.putExtra(BASE_64_IMAGE, strBase64Bitmap);
//                                intent.putExtra(USERNAME, mName);
//                                intent.putExtra(USER_ID, mId[0]);
//                                intent.putExtra(USER_FEATURES, new String(mFeature));
////                                setResult(Activity.RESULT_OK, intent);
////                                TrainingActivity.this.finish();
//
//
//                            } else {
////                                Intent intent = new Intent(context, CreateUserActivity.class);
////                                Intent intent = new Intent();
//                                if (null != bitmap) {
//                                    String strBase64Bitmap = Base64.encodeToString(out.toByteArray(), Base64.DEFAULT);
//                                    intent.putExtra(BASE_64_IMAGE, strBase64Bitmap);
//                                    intent.putExtra(USERNAME, mName);
//                                    intent.putExtra(USER_ID, mId[0]);
//                                    intent.putExtra(USER_FEATURES, new String(mFeature));
//                                }
//
////                                runOnUiThread(() -> {
////                                    TrainingActivity.this.finish();
////                                    startActivity(intent);
////                                });
//
//                            }

                        } catch (Exception e) {
                            e.printStackTrace();

                    }

//                }).start();

                }
//                bm.recycle();
            }

            // mFaceName.setText(mName);


        @SuppressWarnings("NullableProblems")
        @Override
        public void onCaptureProgressed(CameraCaptureSession session, CaptureRequest request,
                CaptureResult partialResult) {
            process(partialResult);
        }

        @SuppressWarnings("NullableProblems")
        @Override
        public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request,
                TotalCaptureResult result) {
            process(result);
            // getFace();

        }

    };
    private Boolean isEditScreen;
    private boolean isSkippable;
    private MenuItem menuSkip;

    /**
     * Given {@code choices} of {@code Size}s supported by a camera, choose the
     * smallest one that is at least as large as the respective texture view size,
     * and that is at most as large as the respective max size, and whose aspect
     * ratio matches with the specified value. If such size doesn't exist, choose
     * the largest one that is at most as large as the respective max size, and
     * whose aspect ratio matches with the specified value.
     *
     * @param choices           The list of sizes that the camera supports for the
     *                          intended output class
     * @param textureViewWidth  The width of the texture view relative to sensor
     *                          coordinate
     * @param textureViewHeight The height of the texture view relative to sensor
     *                          coordinate
     * @param maxWidth          The maximum width that can be chosen
     * @param maxHeight         The maximum height that can be chosen
     * @param aspectRatio       The aspect ratio
     * @return The optimal {@code Size}, or an arbitrary one if none were big enough
     */
    private static Size chooseOptimalSize(Size[] choices, int textureViewWidth, int textureViewHeight, int maxWidth,
            int maxHeight, Size aspectRatio) {

        // Collect the supported resolutions that are at least as big as the preview
        // Surface
        List<Size> bigEnough = new ArrayList<>();
        // Collect the supported resolutions that are smaller than the preview Surface
        List<Size> notBigEnough = new ArrayList<>();
        int w = aspectRatio.getWidth();
        int h = aspectRatio.getHeight();
        for (Size option : choices) {
            if (option.getWidth() <= maxWidth && option.getHeight() <= maxHeight
                    && option.getHeight() == option.getWidth() * h / w) {
                if (option.getWidth() >= textureViewWidth && option.getHeight() >= textureViewHeight) {
                    bigEnough.add(option);
                } else {
                    notBigEnough.add(option);
                }
            }
        }

        // Pick the smallest of those big enough. If there is no one big enough, pick
        // the
        // largest of those not big enough.
        if (bigEnough.size() > 0) {
            return Collections.min(bigEnough, new CompareSizesByArea());
        } else if (notBigEnough.size() > 0) {
            return Collections.max(notBigEnough, new CompareSizesByArea());
        } else {
            AppLogger.e(TAG, "Couldn't find any suitable preview size");
            return choices[0];
        }
    }

    /**
     * Initialize the toolbar
     */
    private void setToolbar() {
        setSupportActionBar(activityTrainingBinding.toolbar);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setDisplayShowHomeEnabled(true);
        activityTrainingBinding.toolbar.setNavigationOnClickListener(view -> finish());
    }

    /**
     * {@link CameraDevice.StateCallback} is called when {@link CameraDevice}
     * changes its state.
     */
    private final CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @SuppressWarnings("NullableProblems")
        @Override
        public void onOpened(CameraDevice cameraDevice) {
            // This method is called when the camera is opened. We start camera preview
            // here.
            mCameraOpenCloseLock.release();
            mCameraDevice = cameraDevice;
            createCameraPreviewSession();
        }

        @Override
        public void onDisconnected(CameraDevice cameraDevice) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(CameraDevice cameraDevice, int error) {
            mCameraOpenCloseLock.release();
            cameraDevice.close();
            mCameraDevice = null;
            finish();
        }

    };

    /**
     * Manage the click listener for start training.
     *
     * @param view instance of click item
     */
    public void onClickStart(View view) {
        Log.e(TAG, ">>onClickStart");
        activityTrainingBinding.getVm().getIsCameraRunning().set(true);
        // menuCamera.setVisible(true);
        setCameraCallback();
        Log.e(TAG, "<<onClickStart");
    }

    public void onClickCancel(View view) {
        finish();
    }

    public void onClickCamera(View view) {
        mOasis.Exit();
        mOasis.Init(useHeavyModel, isElock);
        mState = STATE_FACE_DETECT;
        mFeature = new byte[mOasis.getFaceItemSize()];
        activityTrainingBinding.btnCamera.setVisibility(View.INVISIBLE);
        activityTrainingBinding.heavyModelCheckbox.setVisibility(View.INVISIBLE);
        activityTrainingBinding.radioELock.setVisibility(View.INVISIBLE);
        activityTrainingBinding.radioDoorAccess.setVisibility(View.INVISIBLE);
        // takePicture();
    }

    /**
     * Check and validate the permission based on android version and start the
     * preview.
     */
    public void checkPermission() {
        if (android.os.Build.VERSION.SDK_INT > 23) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED
                    || ContextCompat.checkSelfPermission(this,
                            Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                        new String[] { Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE }, 101);
            } // Permission has already been granted
              // if (ContextCompat.checkSelfPermission(this,
              // Manifest.permission.WRITE_EXTERNAL_STORAGE)
              // != PackageManager.PERMISSION_GRANTED) {
              // ActivityCompat.requestPermissions(this,
              // new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
              // 102);
              // } // Permission has already been granted

        }
    }

    /**
     * This alert shows when user deny the permission.
     */
    private void showAlertForSettingScreen() {
        AlertDialog.Builder builder = new AlertDialog.Builder(TrainingActivity.this);
        builder.setTitle(R.string.title_camera_permission);
        builder.setMessage(R.string.msg_camera_permission);
        builder.setPositiveButton(R.string.ok, (dialogInterface, i) -> {
            Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
            Uri uri = Uri.fromParts("package", getPackageName(), null);
            intent.setData(uri);
            startActivityForResult(intent, 100);
        });
        builder.setNegativeButton(R.string.cancel, (dialogInterface, i) -> finish());
        builder.setCancelable(false);
        AlertDialog dialog = builder.create();
        dialog.show();
        // dialog.getButton(AlertDialog.BUTTON_POSITIVE).setTextColor(getResources().getColor(R.color.colorPrimaryDark));
        // dialog.getButton(AlertDialog.BUTTON_NEGATIVE).setTextColor(getResources().getColor(R.color.colorPrimaryDark));
    }

    /**
     * An additional thread for running tasks that shouldn't block the UI.
     */
    private HandlerThread mBackgroundThread;
    /**
     * A {@link Handler} for running tasks in the background.
     */
    private Handler mBackgroundHandler;
    /**
     * An {@link ImageReader} that handles still image capture.
     */
    private ImageReader mImageReader;
    /**
     * This is the output file for our picture.
     */
    private File mFile;
    /**
     * This a callback object for the {@link ImageReader}. "onImageAvailable" will
     * be called when a still image is ready to be saved.
     */
    private final ImageReader.OnImageAvailableListener mOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {

        @Override
        public void onImageAvailable(ImageReader reader) {
            mBackgroundHandler.post(new ImageSaver(reader.acquireNextImage(), mFile));

            Image image = null;
            try {
                image = reader.acquireLatestImage();
                ByteBuffer buffer = image.getPlanes()[0].getBuffer();
                byte[] bytes = new byte[buffer.capacity()];
                buffer.get(bytes);
                save(bytes);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                if (image != null) {
                    image.close();
                }
            }

        }

        private void save(byte[] bytes) throws IOException {
            OutputStream output = null;
            try {
                output = new FileOutputStream(mFile);
                output.write(bytes);
            } finally {
                if (null != output) {
                    output.close();
                }
            }
        }

    };
    /**
     * {@link CaptureRequest.Builder} for the camera preview
     */
    private CaptureRequest.Builder mPreviewRequestBuilder;
    /**
     * {@link CaptureRequest} generated by {@link #mPreviewRequestBuilder}
     */
    private CaptureRequest mPreviewRequest;
    /**
     * The current state of camera state for taking pictures.
     *
     * @see #mCaptureCallback
     */
    private int mState = STATE_PREVIEW;
    /**
     * A {@link Semaphore} to prevent the app from exiting before closing the
     * camera.
     */
    private Semaphore mCameraOpenCloseLock = new Semaphore(1);
    /**
     * Whether the current camera device supports Flash or not.
     */
    private boolean mFlashSupported;
    /**
     * Orientation of the camera sensor
     */
    private int mSensorOrientation;
    /**
     * Whether to use heavy model or not.
     */
    private int useHeavyModel = 0;
    /**
     * Flag to determine eLock or Door access case.
     */
    private int isElock = 1;



    /**
     * Enable permission callback
     *
     * @param requestCode  requested code
     * @param permissions  enable permission list
     * @param grantResults result
     */
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
            @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case 101:
                if (grantResults.length <= 0 || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                    // permission was not granted
                    if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.CAMERA)) {
                        checkPermission();
                    } else {
                        showAlertForSettingScreen();
                    }
                } // permission was granted :)

                break;
        }
    }

    /**
     * {@link TextureView.SurfaceTextureListener} handles several lifecycle events
     * on a {@link TextureView}.
     */
    private final TextureView.SurfaceTextureListener mSurfaceTextureListener = new TextureView.SurfaceTextureListener() {

        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture texture, int width, int height) {
            openCamera(width, height);
        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture texture, int width, int height) {
            configureTransform(width, height);
        }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture texture) {
            return true;
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture texture) {
        }

    };

    /**
     * Initialize the variable and binding
     *
     * @param savedInstanceState {@link Bundle}
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.e(TAG, ">>onCreate");
        DONE_CAPTURE = false;

        super.onCreate(savedInstanceState);
        mOasis = new Oasis();
        // Write code for intent getting
        TrainingModel trainingModel = ViewModelProviders.of(this).get(TrainingModel.class);
        activityTrainingBinding = DataBindingUtil.setContentView(this, R.layout.activity_training);
        trainingModel.setList(faceDatabase.userDao().getAll().getValue());
        activityTrainingBinding.setVm(trainingModel);
        activityTrainingBinding.setLifecycleOwner(this);

        activityTrainingBinding.getVm().getIsCameraRunning().set(true);

        activityTrainingBinding.getVm().setUserLiveData(faceDatabase);
        activityTrainingBinding.getVm().getUserLiveData().observe(this, new Observer<List<UserInformation>>() {
            @Override
            public void onChanged(List<UserInformation> userInformations) {
                userInformationList = userInformations;
            }
        });

//        OASIS_H = mOasis.getOasisHeight();
//        OASIS_W = mOasis.getOasisWidth();

        mFaceView = findViewById(R.id.box);
        mFaceBox = findViewById(R.id.face);
//-----------------
        ColorMatrix matrix = new ColorMatrix();
        matrix.setSaturation(0);

        ColorMatrixColorFilter filter = new ColorMatrixColorFilter(matrix);
        mFaceView.setColorFilter(filter);
//---------------------
        if (mFile == null) {
            mFile = new File(Environment.getExternalStorageDirectory() + "/picFr.jpg");
        }
        setToolbar();
        checkPermission();

        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            if (extras.containsKey(IS_SKIPPABLE)) {
                isSkippable = extras.getBoolean(IS_SKIPPABLE);
            } else {
                isEditScreen = extras.getBoolean(IS_EDIT_USER);
            }

            if (extras.containsKey(AppConstants.CONNECTION_TYPE)) {
                mConnectionType = extras.getString(AppConstants.CONNECTION_TYPE);
            }

            if (extras.containsKey(AppConstants.SMARTLOCK_IP)) {
                mSmartLockIP = extras.getString(AppConstants.SMARTLOCK_IP);
            }
        }

        Log.d(TAG, "++" + "connection[" + mConnectionType + ":" + mSmartLockIP + "]");

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED
                && ActivityCompat.checkSelfPermission(this,
                        Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(TrainingActivity.this,
                    new String[] { Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE },
                    REQUEST_CAMERA_PERMISSION);
            return;
        }

        if (activityTrainingBinding.getVm().getIsCameraRunning().get()) {
            Log.e(TAG, "Camera running");
        } else {
            Log.e(TAG, "Camera is not running");
        }

//        Switch heavyModelSwitch = (Switch) findViewById(R.id.heavy_model_switch);
//        heavyModelSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
//            @Override
//            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
//                if (b) {
//                    mOasis.Exit();
//                    mOasis.Init(1);
//                } else {
//                    mOasis.Exit();
//                    mOasis.Init(0);
//                }
//            }
//        });

        CheckBox heavyModelCheckBox = (CheckBox) findViewById(R.id.heavy_model_checkbox);
        heavyModelCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
                if (isChecked) {
                    useHeavyModel = 1;
                } else {
                    useHeavyModel = 0;
                }
//                mOasis.Exit();
//                mOasis.Init(useHeavyModel);
            }
        });

        RadioButton radioELock = (RadioButton) findViewById(R.id.radio_eLock);
        radioELock.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
                if (isChecked) {
                    isElock = 1;
                } else {
                    isElock = 0;
                }
            }
        });

        Log.e(TAG, "<<onCreate");
    }

    /**
     * Display dialog for storing trained user name.
     */
    private void showDialogForUserName() {
        LayoutInflater li = LayoutInflater.from(this);
        @SuppressLint("InflateParams")
        View dialogView = li.inflate(R.layout.username_dialog, null);
        android.app.AlertDialog.Builder alertDialogBuilder = new android.app.AlertDialog.Builder(this);
        // set title
        alertDialogBuilder.setTitle(R.string.face_recognition_name);
        // set custom dialog icon
        alertDialogBuilder.setMessage(R.string.please_enter_unique_user_name_for_the_face_recognition);
        alertDialogBuilder.setIcon(R.drawable.ic_man_user_colored);
        // set custom_dialog.xml to alert dialog builder
        alertDialogBuilder.setView(dialogView);
        final EditText userInput = dialogView.findViewById(R.id.et_input);
        // set dialog message
        alertDialogBuilder.setCancelable(false).setPositiveButton(getString(R.string.ok), (dialog, id) -> {
            // Put here database name validation.
            String username = userInput.getText().toString();
            // if (TextUtils.isEmpty(username)) {
            // showToast("Please enter name");
            // } else if
            // (activityTrainingBinding.getVm().isSameNameExitsInDatabase(username)) {
            // showToast(username + " is already exits.");
            // } else {
            // activityTrainingBinding.getVm().getUserName().set(username);
            // UserInformation userInformation = new UserInformation();
            // userInformation.setName(activityTrainingBinding.getVm().getUserName().get());
            // userInformation.setTrainingData(activityTrainingBinding.getVm().getUserInfo().get());
            // userInformation.setDate(System.currentTimeMillis());
            // faceDatabase.userDao().insertUserInfo(userInformation);
            // UserInformation userInformation1 =
            // faceDatabase.userDao().getByName(username);
            // AppLogger.d("User Information"+ userInformation1.getName());
            // }
        }).setNegativeButton(getString(R.string.cancel), (dialog, id) -> {
            dialog.cancel();
            finish();
        });
        // create alert dialog
        AlertDialog alertDialog = alertDialogBuilder.create();
        // show it
        alertDialog.show();
    }

    /**
     * set the Camera view.
     */
    private void setCameraCallback() {
        mTextureView = activityTrainingBinding.camera;
        startBackgroundThread();
        if (mTextureView.isAvailable()) {
            openCamera(mTextureView.getWidth(), mTextureView.getHeight());
        } else {
            mTextureView.setSurfaceTextureListener(mSurfaceTextureListener);
        }
    }

    /**
     * Bind menu with current view {@link Menu}
     *
     * @param menu {@link Menu}
     * @return true/false
     */
    /*
     * @Override public boolean onCreateOptionsMenu(Menu menu) {
     * getMenuInflater().inflate(R.menu.training_menu, menu); menuCamera =
     * menu.findItem(R.id.menu_camera); menuSkip = menu.findItem(R.id.menu_skip); if
     * (activityTrainingBinding.getVm().getIsCameraRunning().get()) {
     * menuCamera.setVisible(true); } else { menuCamera.setVisible(false); }
     * 
     * if (isSkippable) { menuSkip.setVisible(true); } else {
     * menuSkip.setVisible(false); } return true; }
     */

    /**
     * Menu item select listener {@link MenuItem}.
     *
     * @param item {@link MenuItem}
     * @return true/false
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_camera:
                takePicture();
                break;

            case R.id.menu_skip:
                /*
                 * finish(); Intent intent = new Intent(context, CreateUserActivity.class);
                 * startActivity(intent);
                 */
                break;
        }
        return true;
    }

    /**
     * Device comes from background to foreground and set camera callback again.
     */
    @Override
    protected void onResume() {
        Log.e(TAG, ">>onResume");
        super.onResume();
        int ret = mOasis.Init(useHeavyModel, isElock);
        OASIS_H = mOasis.getOasisHeight();
        OASIS_W = mOasis.getOasisWidth();
        if (activityTrainingBinding.getVm().getIsCameraRunning().get()) {
            Log.e(TAG, "Camera running");
            // if (menuCamera != null) menuCamera.setVisible(true);
            setCameraCallback();
        }
        Log.e(TAG, "<<onResume");
    }

    /**
     * Device goes into background.
     */
    @Override
    protected void onPause() {
        mState = STATE_DETECTION_DONE;
        if (activityTrainingBinding.getVm() != null && activityTrainingBinding.getVm().getIsCameraRunning().get()) {
            closeCamera();
            mOasis.cancelOp();
            stopBackgroundThread();
            // menuCamera.setVisible(false);
        }
        super.onPause();

    }

    /**
     * Set the camera result based on ratio and facing side.
     *
     * @param width  View width
     * @param height View height
     */
    private void setUpCameraOutputs(int width, int height) {
        Activity activity = this;
        CameraManager manager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        try {
            if (manager != null) {
                for (String cameraId : manager.getCameraIdList()) {
                    CameraCharacteristics characteristics = manager.getCameraCharacteristics(cameraId);

                    // We don't use a front facing camera in this sample.
                    Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);
                    if (facing != null && facing == CameraCharacteristics.LENS_FACING_FRONT) {
                        // continue;
                        // }

                        StreamConfigurationMap map = characteristics
                                .get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                        if (map == null) {
                            continue;
                        }

                        // For still image captures, we use the largest available size.
                        Size largest = Collections.max(Arrays.asList(map.getOutputSizes(ImageFormat.JPEG)),
                                new CompareSizesByArea());
                        mImageReader = ImageReader.newInstance(largest.getWidth(), largest.getHeight(),
                                ImageFormat.JPEG, /* maxImages */2);
                        mImageReader.setOnImageAvailableListener(mOnImageAvailableListener, mBackgroundHandler);

                        // Find out if we need to swap dimension to get the preview size relative to
                        // sensor
                        // coordinate.
                        int displayRotation = activity.getWindowManager().getDefaultDisplay().getRotation();
                        // noinspection ConstantConditions
                        mSensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
                        boolean swappedDimensions = false;
                        switch (displayRotation) {
                            case Surface.ROTATION_0:
                            case Surface.ROTATION_180:
                                if (mSensorOrientation == 90 || mSensorOrientation == 270) {
                                    swappedDimensions = true;
                                }
                                break;
                            case Surface.ROTATION_90:
                            case Surface.ROTATION_270:
                                if (mSensorOrientation == 0 || mSensorOrientation == 180) {
                                    swappedDimensions = true;
                                }
                                break;
                            default:
                                AppLogger.e(TAG, "Display rotation is invalid: " + displayRotation);
                        }

                        Point displaySize = new Point();
                        activity.getWindowManager().getDefaultDisplay().getSize(displaySize);
                        int rotatedPreviewWidth = width;
                        int rotatedPreviewHeight = height;
                        int maxPreviewWidth = displaySize.x;
                        int maxPreviewHeight = displaySize.y;

                        if (swappedDimensions) {
                            rotatedPreviewWidth = height;
                            rotatedPreviewHeight = width;
                            maxPreviewWidth = displaySize.y;
                            maxPreviewHeight = displaySize.x;
                        }

                        if (maxPreviewWidth > MAX_PREVIEW_WIDTH) {
                            maxPreviewWidth = MAX_PREVIEW_WIDTH;
                        }

                        if (maxPreviewHeight > MAX_PREVIEW_HEIGHT) {
                            maxPreviewHeight = MAX_PREVIEW_HEIGHT;
                        }

                        // Danger, W.R.! Attempting to use too large a preview size could exceed the
                        // camera
                        // bus' bandwidth limitation, resulting in gorgeous previews but the storage of
                        // garbage capture data.
                        mPreviewSize = chooseOptimalSize(map.getOutputSizes(SurfaceTexture.class), rotatedPreviewWidth,
                                rotatedPreviewHeight, maxPreviewWidth, maxPreviewHeight, largest);

                        // We fit the aspect ratio of TextureView to the size of preview we picked.
                        int orientation = getResources().getConfiguration().orientation;
                        if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
                            mTextureView.setAspectRatio(mPreviewSize.getWidth(), mPreviewSize.getHeight());
                        } else {
                            mTextureView.setAspectRatio(mPreviewSize.getHeight(), mPreviewSize.getWidth());
                        }

                        // Check if the flash is supported.
                        Boolean available = characteristics.get(CameraCharacteristics.FLASH_INFO_AVAILABLE);
                        mFlashSupported = available == null ? false : available;

                        mCameraId = cameraId;
                        return;
                    }
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {
            // Currently an NPE is thrown when the Camera2API is used but not supported on
            // the
            // device this code runs.
            Toast.makeText(this, "Error", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * Opens the camera specified by {@link}.
     */
    private void openCamera(int width, int height) {

        setUpCameraOutputs(width, height);
        configureTransform(width, height);
        Activity activity = this;
        CameraManager manager = (CameraManager) activity.getSystemService(Context.CAMERA_SERVICE);
        try {
            if (ActivityCompat.checkSelfPermission(this,
                    Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                // TODO: Consider calling
                return;
            }
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }

            if (manager != null) {
                manager.openCamera(mCameraId, mStateCallback, mBackgroundHandler);
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera opening.", e);
        }
    }

    /**
     * Closes the current {@link CameraDevice}.
     */
    private void closeCamera() {
        try {
            mCameraOpenCloseLock.acquire();
            if (null != mCaptureSession) {
                mCaptureSession.close();
                mCaptureSession = null;
            }
            if (null != mCameraDevice) {
                mCameraDevice.close();
                mCameraDevice = null;
            }
            if (null != mImageReader) {
                mImageReader.close();
                mImageReader = null;
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing.", e);
        } finally {
            mCameraOpenCloseLock.release();
        }
    }

    /**
     * Starts a background thread and its {@link Handler}.
     */
    private void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("CameraBackground");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    /**
     * Stops the background thread and its {@link Handler}.
     */
    private void stopBackgroundThread() {
        mBackgroundThread.quitSafely();
        try {
            mBackgroundThread.join();
            mBackgroundThread = null;
            mBackgroundHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    /**
     * Creates a new {@link CameraCaptureSession} for camera preview.
     */
    private void createCameraPreviewSession() {
        try {
            SurfaceTexture texture = mTextureView.getSurfaceTexture();
            assert texture != null;

            // We configure the size of default buffer to be the size of camera preview we
            // want.
            texture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());

            // This is the output Surface we need to start preview.
            Surface surface = new Surface(texture);

            // We set up a CaptureRequest.Builder with the output Surface.
            mPreviewRequestBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            mPreviewRequestBuilder.addTarget(surface);

            // Here, we create a CameraCaptureSession for camera preview.
            mCameraDevice.createCaptureSession(Arrays.asList(surface, mImageReader.getSurface()),
                    new CameraCaptureSession.StateCallback() {
                        @SuppressWarnings("NullableProblems")
                        @Override
                        public void onConfigured(CameraCaptureSession cameraCaptureSession) {
                            // The camera is already closed
                            if (null == mCameraDevice) {
                                return;
                            }

                            // When the session is ready, we start displaying the preview.
                            mCaptureSession = cameraCaptureSession;
                            try {
                                // Auto focus should be continuous for camera preview.
                                mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                                        CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                                // Flash is automatically enabled when necessary.
                                setAutoFlash(mPreviewRequestBuilder);

                                // Finally, we start displaying the camera preview.
                                mPreviewRequest = mPreviewRequestBuilder.build();
                                mCaptureSession.setRepeatingRequest(mPreviewRequest, mCaptureCallback,
                                        mBackgroundHandler);
                            } catch (CameraAccessException e) {
                                e.printStackTrace();
                            }
                        }

                        @SuppressWarnings("NullableProblems")
                        @Override
                        public void onConfigureFailed(CameraCaptureSession cameraCaptureSession) {
                            showToast("Failed");
                        }
                    }, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Configures the necessary {@link Matrix} transformation to `mTextureView`.
     * This method should be called after the camera preview size is determined in
     * setUpCameraOutputs and also the size of `mTextureView` is fixed.
     *
     * @param viewWidth  The width of `mTextureView`
     * @param viewHeight The height of `mTextureView`
     */
    private void configureTransform(int viewWidth, int viewHeight) {
        Activity activity = this;
        if (null == mTextureView || null == mPreviewSize) {
            return;
        }
        int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
        Matrix matrix = new Matrix();
        RectF viewRect = new RectF(0, 0, viewWidth, viewHeight);
        RectF bufferRect = new RectF(0, 0, mPreviewSize.getHeight(), mPreviewSize.getWidth());
        float centerX = viewRect.centerX();
        float centerY = viewRect.centerY();
        if (Surface.ROTATION_90 == rotation || Surface.ROTATION_270 == rotation) {
            bufferRect.offset(centerX - bufferRect.centerX(), centerY - bufferRect.centerY());
            matrix.setRectToRect(viewRect, bufferRect, Matrix.ScaleToFit.FILL);
            float scale = Math.max((float) viewHeight / mPreviewSize.getHeight(),
                    (float) viewWidth / mPreviewSize.getWidth());
            matrix.postScale(scale, scale, centerX, centerY);
            matrix.postRotate(90 * (rotation - 2), centerX, centerY);
        } else if (Surface.ROTATION_180 == rotation) {
            matrix.postRotate(180, centerX, centerY);
        }
        mTextureView.setTransform(matrix);
    }

    /**
     * Initiate a still image capture.
     */
    private void takePicture() {
        // lockFocus();
        capturePicture();
    }

    /**
     * Start recording and face recognizing. Terminate when user's front face with
     * sufficient features is captured.
     */

    private void record() {
        Log.e(TAG, ">>recording");
        if (null == mCameraDevice) {
            Log.e(TAG, "camera is null");
            return;
        }

    }

    private void capturePicture() {
        Log.e(TAG, ">>capturePicture");
        if (null == mCameraDevice) {
            Log.e(TAG, "cameraDevice is null");
            return;
        }
        CameraManager manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        try {
            CameraCharacteristics characteristics = manager.getCameraCharacteristics(mCameraDevice.getId());
            Size[] jpegSizes = null;
            if (characteristics != null) {
                jpegSizes = Objects
                        .requireNonNull(characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP))
                        .getOutputSizes(ImageFormat.JPEG);
            }
            int width = 640;
            int height = 480;
            int support720p = 0;
            int supportvga = 0;
            if (jpegSizes != null) {
                for (int i = 0; i < jpegSizes.length; i++) {
                    Log.e(TAG,
                            "supported jpeg size:[" + jpegSizes[i].getWidth() + ":" + jpegSizes[i].getHeight() + "]");
                    if (jpegSizes[i].getWidth() == 640 && jpegSizes[i].getHeight() == 480) {
                        supportvga = 1;
                    } else if (jpegSizes[i].getWidth() == 1280 && jpegSizes[i].getHeight() == 720) {
                        support720p = 1;
                    }
                }
            }

            if (supportvga == 1) {
                width = 640;
                height = 480;
            } else if (support720p == 1) {
                width = 1280;
                height = 720;
            } else if (jpegSizes != null) {
                width = jpegSizes[0].getWidth();
                height = jpegSizes[0].getHeight();
            } else {
                Log.e(TAG, "No suitable jpeg resolution");
                return;
            }

            Log.e(TAG, "Capture Size:[" + width + "x" + height + "]");

            ImageReader reader = ImageReader.newInstance(width, height, ImageFormat.JPEG, 1);
            List<Surface> outputSurfaces = new ArrayList<Surface>(2);
            outputSurfaces.add(reader.getSurface());
            // outputSurfaces.add(new Surface(textureView.getSurfaceTexture()));
            final CaptureRequest.Builder captureBuilder = mCameraDevice
                    .createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
            captureBuilder.addTarget(reader.getSurface());
            captureBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);
            // Orientation
            int rotation = getWindowManager().getDefaultDisplay().getRotation();
            Log.e(TAG, "ROTATION" + rotation);
            // captureBuilder.set(CaptureRequest.JPEG_ORIENTATION,
            // ORIENTATIONS.get(rotation));
            captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, getOrientation(rotation));
            DONE_CAPTURE = true;
            final File file = new File(Environment.getExternalStorageDirectory() + "/picFR.jpg");
            ImageReader.OnImageAvailableListener readerListener = new ImageReader.OnImageAvailableListener() {
                @Override
                public void onImageAvailable(ImageReader reader) {
                    Image image = null;
                    try {
                        image = reader.acquireLatestImage();
                        ByteBuffer buffer = image.getPlanes()[0].getBuffer();
                        byte[] bytes = new byte[buffer.capacity()];
                        buffer.get(bytes);
                        save(bytes);
                    } catch (FileNotFoundException e) {
                        e.printStackTrace();
                    } catch (IOException e) {
                        e.printStackTrace();
                    } finally {
                        if (image != null) {
                            image.close();
                        }
                    }
                }

                private void save(byte[] bytes) throws IOException {
                    OutputStream output = null;
                    try {
                        output = new FileOutputStream(file);
                        output.write(bytes);
                    } finally {
                        if (null != output) {
                            output.close();
                            new Thread(() -> {
                                if (mFile != null) {
                                    try {

                                        /*
                                         * Bitmap bitmap = BitmapFactory.decodeFile(mFile.toString()); Bitmap resized =
                                         * Bitmap.createScaledBitmap(bitmap, 100, 100, true); Matrix matrix = new
                                         * Matrix(); matrix.postRotate(-90); Bitmap rotatedBitmap =
                                         * Bitmap.createBitmap(resized, 0, 0, resized.getWidth(), resized.getHeight(),
                                         * matrix, true); //need to check with rajesh when to enable below line //
                                         * activityTrainingBinding.getVm().getUserPhoto().set(value);
                                         * ByteArrayOutputStream out = new ByteArrayOutputStream();
                                         * rotatedBitmap.compress(Bitmap.CompressFormat.PNG, 50, out);
                                         * 
                                         * 
                                         * if (!isSkippable) { Intent resultIntent = new Intent(); String
                                         * strBase64Bitmap = Base64.encodeToString(out.toByteArray(), Base64.DEFAULT);
                                         * resultIntent.putExtra(BASE_64_IMAGE, strBase64Bitmap);
                                         * setResult(Activity.RESULT_OK, resultIntent); TrainingActivity.this.finish();
                                         * 
                                         * 
                                         * } else { Intent intent = new Intent(context, CreateUserActivity.class); if
                                         * (null != bitmap) { String strBase64Bitmap =
                                         * Base64.encodeToString(out.toByteArray(), Base64.DEFAULT);
                                         * 
                                         * intent.putExtra(BASE_64_IMAGE, strBase64Bitmap); } runOnUiThread(() -> {
                                         * TrainingActivity.this.finish(); startActivity(intent); });
                                         * 
                                         * }
                                         */
                                        // TrainingActivity.this.finish();
//                                        Intent intent = new Intent(context, AddFaceActivity.class);
                                        // Intent intent = new Intent(getActivity(), DeviceScanActivity.class);
                                        String filePath = Environment.getExternalStorageDirectory() + "/picFR.jpg";
//                                        intent.putExtra("filePath", filePath);
//                                        intent.putExtra(AppConstants.CONNECTION_TYPE, mConnectionType);
//                                        intent.putExtra(AppConstants.SMARTLOCK_IP, mSmartLockIP);
//
//                                        startActivity(intent);
                                    } catch (Exception e) {
                                        e.printStackTrace();
                                    }
                                }

                            }).start();
                        }
                    }
                }
            };
            reader.setOnImageAvailableListener(readerListener, mBackgroundHandler);
            final CameraCaptureSession.CaptureCallback captureListener = new CameraCaptureSession.CaptureCallback() {
                @Override
                public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request,
                        TotalCaptureResult result) {
                    super.onCaptureCompleted(session, request, result);
                    // Toast.makeText(TrainingActivity.this, "Saved:" + file,
                    // Toast.LENGTH_SHORT).show();
                    createCameraPreviewSession();
                }
            };
            mCameraDevice.createCaptureSession(outputSurfaces, new CameraCaptureSession.StateCallback() {
                @Override
                public void onConfigured(CameraCaptureSession session) {
                    try {
                        session.capture(captureBuilder.build(), captureListener, mBackgroundHandler);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void onConfigureFailed(CameraCaptureSession session) {
                }
            }, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        Log.e(TAG, "<<capturePicture");
    }

    /**
     * Lock the focus as the first step for a still image capture.
     */
    private void lockFocus() {
        try {
            // This is how to tell the camera to lock focus.
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER, CameraMetadata.CONTROL_AF_TRIGGER_START);
            // Tell #mCaptureCallback to wait for the lock.
            mState = STATE_WAITING_LOCK;
            mCaptureSession.capture(mPreviewRequestBuilder.build(), mCaptureCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Run the precapture sequence for capturing a still image. This method should
     * be called when we get a response in {@link #mCaptureCallback} from
     * {@link #lockFocus()}.
     */
    private void runPrecaptureSequence() {
        try {
            // This is how to tell the camera to trigger.
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                    CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START);
            // Tell #mCaptureCallback to wait for the precapture sequence to be set.
            mState = STATE_WAITING_PRECAPTURE;
            mCaptureSession.capture(mPreviewRequestBuilder.build(), mCaptureCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Capture a still picture. This method should be called when we get a response
     * in {@link #mCaptureCallback} from both {@link #lockFocus()}.
     */
    private void captureStillPicture() {
        try {
            final Activity activity = this;
            if (null == mCameraDevice) {
                return;
            }
            // This is the CaptureRequest.Builder that we use to take a picture.
            final CaptureRequest.Builder captureBuilder = mCameraDevice
                    .createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
            captureBuilder.addTarget(mImageReader.getSurface());

            // Use the same AE and AF modes as the preview.
            captureBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
            setAutoFlash(captureBuilder);

            // Orientation
            int rotation = activity.getWindowManager().getDefaultDisplay().getRotation();
            captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, getOrientation(rotation));

            CameraCaptureSession.CaptureCallback CaptureCallback = new CameraCaptureSession.CaptureCallback() {
                @SuppressWarnings("NullableProblems")
                @Override
                public void onCaptureCompleted(CameraCaptureSession session, CaptureRequest request,
                        TotalCaptureResult result) {
                    // showToast("Saved: " + mFile);
                    AppLogger.d("Saved File name" + mFile.toString());
                    unlockFocus();
                    new Thread(() -> {
                        if (mFile != null) {
                            try {
                                Bitmap bitmap = BitmapFactory.decodeFile(mFile.toString());
                                Bitmap resized = Bitmap.createScaledBitmap(bitmap, 480, 320, true);
                                // need to check with rajesh when to enable below line
                                // activityTrainingBinding.getVm().getUserPhoto().set(value);
                                // ByteArrayOutputStream out = new ByteArrayOutputStream();
                                // resized.compress(Bitmap.CompressFormat.PNG, 50, out);
                                // Bitmap decoded = BitmapFactory.decodeStream(new
                                // ByteArrayInputStream(out.toByteArray()));

                                if (!isSkippable) {
                                    Intent resultIntent = new Intent();
                                    String strBase64Bitmap = "";
                                    resultIntent.putExtra(BASE_64_IMAGE, strBase64Bitmap);
                                    setResult(Activity.RESULT_OK, resultIntent);
                                } else {

                                    Intent intent = new Intent(context, CreateUserActivity.class);
                                    startActivity(intent);
                                }
                                TrainingActivity.this.finish();

                            } catch (Exception e) {
                                e.printStackTrace();
                            }
                        }

                    }).start();

                }
            };

            mCaptureSession.stopRepeating();
            mCaptureSession.abortCaptures();
            mCaptureSession.capture(captureBuilder.build(), CaptureCallback, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Retrieves the JPEG orientation from the specified screen rotation.
     *
     * @param rotation The screen rotation.
     * @return The JPEG orientation (one of 0, 90, 270, and 360)
     */
    private int getOrientation(int rotation) {
        // Sensor orientation is 90 for most devices, or 270 for some devices (eg. Nexus
        // 5X)
        // We have to take that into account and rotate JPEG properly.
        // For devices with orientation of 90, we simply return our mapping from
        // ORIENTATIONS.
        // For devices with orientation of 270, we need to rotate the JPEG 180 degrees.
        return (ORIENTATIONS.get(rotation) + mSensorOrientation + 270) % 360;
    }

    /**
     * Unlock the focus. This method should be called when still image capture
     * sequence is finished.
     */
    private void unlockFocus() {
        try {
            // Reset the auto-focus trigger
            mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_TRIGGER, CameraMetadata.CONTROL_AF_TRIGGER_CANCEL);
            setAutoFlash(mPreviewRequestBuilder);
            mCaptureSession.capture(mPreviewRequestBuilder.build(), mCaptureCallback, mBackgroundHandler);
            // After this, the camera will go back to the normal state of preview.
            mState = STATE_PREVIEW;
            mCaptureSession.setRepeatingRequest(mPreviewRequest, mCaptureCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    /**
     * Set the autoflash option.
     *
     * @param requestBuilder {@link CaptureRequest}
     */
    private void setAutoFlash(CaptureRequest.Builder requestBuilder) {
        if (mFlashSupported) {
            requestBuilder.set(CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH);
        }
    }

    /**
     * Saves a JPEG {@link Image} into the specified {@link File}.
     */
    private static class ImageSaver implements Runnable {

        /**
         * The JPEG image
         */
        private final Image mImage;
        /**
         * The file we save the image into.
         */
        private final File mFile;

        ImageSaver(Image image, File file) {
            mImage = image;
            mFile = file;
        }

        @Override
        public void run() {
            ByteBuffer buffer = mImage.getPlanes()[0].getBuffer();
            byte[] bytes = new byte[buffer.remaining()];
            buffer.get(bytes);
            FileOutputStream output = null;
            try {
                output = new FileOutputStream(mFile);
                output.write(bytes);
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                mImage.close();
                if (null != output) {
                    try {
                        output.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

    }

    /**
     * Compares two {@code Size}s based on their areas.
     */
    static class CompareSizesByArea implements Comparator<Size> {

        @Override
        public int compare(Size lhs, Size rhs) {
            // We cast here to ensure the multiplications won't overflow
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() - (long) rhs.getWidth() * rhs.getHeight());
        }

    }

    @Override
    public boolean onSupportNavigateUp() {
        onBackPressed();
        mOasis.cancelOp();
        return true;
    }
}

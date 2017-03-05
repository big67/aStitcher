package com.example.wrapper.tracking;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.util.Log;

import org.opencv.imgproc.Imgproc;
import org.opencv.core.Core;
import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.core.Point;

import java.util.concurrent.atomic.AtomicReference;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.Color;
import android.view.SurfaceHolder;
import android.view.View.OnTouchListener;
import android.view.View;
import android.view.MotionEvent;
import org.opencv.core.Rect;
import android.graphics.Canvas;
import android.graphics.PorterDuff;

import org.opencv.core.Scalar;

import com.example.wrapper.stitch.Stitcher;

public class MainActivity extends AppCompatActivity implements CvCameraViewListener2 {

    int frameCnt;

    public native boolean setupTacking(long matAddrGr, long x, long y, long w, long h);
    public native void initTracking(int width, int height);
    public native void process(long matAddrGray);
    public native int[] getRect();
    public native void cleanup();

    static final int WIDTH = 120;
    static final int HEIGHT = 120;
    private static final int START = 2;
    private static final int TRACK = 1;
    private static final int NOTHING = 0;

    static boolean uno = true;

    private static final String    TAG = "SRLog";
    private CameraBridgeViewBase cvCamView;

    private int mViewMode = NOTHING;

    private int _ImgYOffset;
    private int _ImgXOffset;
    private Rect _trackedArea = null;

    SurfaceHolder _holder;

    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("native-lib");

                    try{
                        //test tracker
//                        initTracking(WIDTH, HEIGHT);
//                        cvCamView.enableView();

                        // test stitcher
                        testStiticher();

                    }catch (Exception e){
                        Log.e("SRLog", e.toString());
                    }
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        // Example of a call to a native method
//        TextView tv = (TextView) findViewById(R.id.sample_text);
//        tv.setText(stringFromJNI());


        //test tracker
//        testTracker();


    }

    private void testStiticher(){
        Stitcher stitcher = new Stitcher();
        stitcher.test(this.getApplicationContext());
    }

    private void testTracker(){

        cvCamView = (CameraBridgeViewBase) findViewById(R.id.tutorial1_activity_java_surface_view);

        cvCamView.setVisibility(SurfaceView.VISIBLE);

        cvCamView.setCvCameraViewListener(this);


        final AtomicReference<Point> trackedBox1stCorner = new AtomicReference<Point>();
        final Paint rectPaint = new Paint();
        rectPaint.setColor(Color.rgb(0, 0, 255));
        rectPaint.setStrokeWidth(5);
        rectPaint.setStyle(Style.STROKE);

        _holder = cvCamView.getHolder();
        cvCamView.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                // re-init
                final Point corner = new Point(
                        event.getX() - _ImgXOffset, event.getY()
                        - _ImgYOffset);
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        stopTracking();
                        trackedBox1stCorner.set(corner);
                        break;
                    case MotionEvent.ACTION_UP:
                        _trackedArea = new Rect(trackedBox1stCorner.get(), corner);
                        if (_trackedArea.area() > 100) {
                            mViewMode = START;
                        } else
                            _trackedArea = null;
                        break;
                    case MotionEvent.ACTION_MOVE:
                        final android.graphics.Rect rect = new android.graphics.Rect(
                                (int) trackedBox1stCorner.get().x
                                        + _ImgXOffset,
                                (int) trackedBox1stCorner.get().y
                                        + _ImgYOffset, (int) corner.x
                                + _ImgXOffset, (int) corner.y
                                + _ImgYOffset);
                        final Canvas canvas = _holder.lockCanvas(rect);
                        canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
                        // remove old rectangle
                        canvas.drawRect(rect, rectPaint);
                        _holder.unlockCanvasAndPost(canvas);
                }
                return true;
            }
        });


    }

    private void stopTracking()
    {
        mViewMode = NOTHING;
        uno = true;
        _trackedArea = null;

    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();


    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onStop() {
        super.onStop();
    }

    @Override
    public void onPause()
    {
        super.onPause();
        if (cvCamView != null)
            cvCamView.disableView();
    }

    @Override
    public void onResume()
    {
        super.onResume();

        if (!OpenCVLoader.initDebug()) {
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_13, this, mLoaderCallback);
        } else {
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    public void onDestroy() {
        super.onDestroy();
        if (cvCamView != null)
            cvCamView.disableView();
    }

    public void onCameraViewStarted(int width, int height) {
    }

    public void onCameraViewStopped() {
        cleanup();
    }

    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {


        final int viewMode = mViewMode;
        Mat mRgba = inputFrame.rgba();

        switch (viewMode) {
            case NOTHING:
//                mGray = inputFrame.gray();
//                FindFeatures(mGray.getNativeObjAddr(), mRgba.getNativeObjAddr());
                break;
            case START: {
                mRgba = inputFrame.rgba();
                Mat mGray = reSize(inputFrame.gray());
                double w = mGray.width();
                double h = mGray.height();
                boolean isInit;

                if (_trackedArea == null)

                    isInit = setupTacking(mGray.getNativeObjAddr(),
                            (long) (w / 2 - w / 4), (long) (h / 2 - h / 4),
                            (long) w / 2, (long) h / 2);
                else {

                    double px = (w) / (double) (cvCamView.getWidth());
                    double py = (h) / (double) (cvCamView.getHeight());
                    //
                    isInit = setupTacking(mGray.getNativeObjAddr(),
                            (long) (_trackedArea.x * px),
                            (long) (_trackedArea.y * py),
                            (long) (_trackedArea.width * px),
                            (long) (_trackedArea.height * py));
                }

                mGray.release();

                if(isInit) {
                    uno = false;
                    mViewMode = TRACK;
                }
            }
            break;
            case TRACK:
            {
                mRgba = inputFrame.rgba();
                Mat mGray = inputFrame.gray();
                mGray = reSize(mGray);

                if (frameCnt % 5 != 0){
                    process(mGray.getNativeObjAddr());
                }
                frameCnt++;

                double px = (double) mRgba.width() / (double) mGray.width();
                double py = (double) mRgba.height() / (double) mGray.height();
                int[] l = getRect();
                if (l != null) {
                        Point p0 = new Point(l[0]*px, l[1]*py);
                        Point p1 = new Point(l[2]*px, l[3]*py);
                        Point p2 = new Point(l[4]*px, l[5]*py);
                        Point p3 = new Point(l[6]*px, l[7]*py);

//                    Core.circle(mRgba, p0, 10, new Scalar(255,0,0));
//                    Core.circle(mRgba, p1, 10, new Scalar(0,255,0));
//                    Core.circle(mRgba, p2, 10, new Scalar(0,0,255));
//                    Core.circle(mRgba, p3, 10, new Scalar(255,255,255));

                        Core.line(mRgba, p0, p1, new Scalar(255,0,0), 2);
                        Core.line(mRgba, p1, p2, new Scalar(0,255,0), 2);
                        Core.line(mRgba, p2, p3, new Scalar(0,0,255), 2);
                        Core.line(mRgba, p3, p0, new Scalar(255,255,255), 2);

//                    Core.circle(mRgba, new Point((l[0]+l[6])*px/2, (l[1]+l[5])*py/2), 10, new Scalar(255,0,0));

                }

                mGray.release();
            }


            break;
        }

        return mRgba;
    }

    Mat reSize(Mat m) {
        Mat dst = new Mat();
        Imgproc.resize(m, dst, new org.opencv.core.Size(WIDTH, HEIGHT));
        m.release();
        return dst;
    }

//    @Override
//    public void onConfigurationChanged(Configuration newConfig){
//        super.onConfigurationChanged(newConfig);
//        stopTracking();
//        cleanup();
//    }


}

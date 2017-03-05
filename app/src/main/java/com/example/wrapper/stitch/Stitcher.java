package com.example.wrapper.stitch;

/**
 * Created by a on 16/12/13.
 */

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.content.Context;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import android.content.res.AssetManager;
import android.os.Environment;
import android.util.Log;

import org.opencv.android.Utils;
import org.opencv.core.CvException;
import org.opencv.core.Mat;

public class Stitcher {

    static String TAG="Stitcher";
   public native void stitch(long images[], long MatAddr);

   public void test(Context ctx){

       String imageName[] ={"1.png","2.png","3.png","4.png","5.png"};
       ArrayList<Mat> matArr = new ArrayList<Mat>(imageName.length);
       long [] matAddr = new long[imageName.length];

       for(int i=0; i<imageName.length; i++){

           Bitmap bitmap= getImageFromAssetsFile(ctx, imageName[i]);

           Mat mat  = new Mat();
           Utils.bitmapToMat(bitmap, mat);
           matArr.add(i, mat);
           matAddr[i] = mat.getNativeObjAddr();

       }

       Mat pano = new Mat();
       stitch(matAddr, pano.getNativeObjAddr());

       Bitmap bmp = null;
       try {
           bmp = Bitmap.createBitmap(pano.cols(), pano.rows(), Bitmap.Config.ARGB_8888);
           Utils.matToBitmap(pano, bmp);
       } catch (CvException e) {
           Log.d(TAG, e.getMessage());
       }

       File sd = new File(Environment.getExternalStorageDirectory() + "/pano");
       boolean success = true;
       if (!sd.exists()) {
           success = sd.mkdir();
       }
       if (success) {
           File dest = new File(sd, "pano.png");
           FileOutputStream out=null;
           try {
               out = new FileOutputStream(dest);
               bmp.compress(Bitmap.CompressFormat.PNG, 100, out); // bmp is your Bitmap instance
               // PNG is a lossless format, the compression factor (100) is ignored

           } catch (Exception e) {
               e.printStackTrace();
               Log.d(TAG, e.getMessage());
           } finally {
               try {
                   if (out != null) {
                       out.close();
                       Log.d(TAG, "OK!!");
                   }
               } catch (IOException e) {
                   Log.d(TAG, e.getMessage() + "Error");
                   e.printStackTrace();
               }
           }
       }

   }

    private Bitmap getImageFromAssetsFile(Context ctx, String fileName)
    {
        Bitmap image = null;
        AssetManager am = ctx.getResources().getAssets();
        try
        {
            InputStream is = am.open(fileName);
            image = BitmapFactory.decodeStream(is);
            is.close();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }

        return image;

    }

}

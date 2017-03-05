//
// Created by a on 16/12/13.
//

#include "stitcher.h"

#include <android/bitmap.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <android/log.h>

#define  LOG_TAG    "SRLog"

#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

using namespace std;

jobject mat_to_bitmap(JNIEnv * env, Mat & src, bool needPremultiplyAlpha, jobject bitmap_config){

    jclass java_bitmap_class = (jclass)env->FindClass("android/graphics/Bitmap");
    jmethodID mid = env->GetStaticMethodID(java_bitmap_class,
                                           "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");

    jobject bitmap = env->CallStaticObjectMethod(java_bitmap_class,
                                                 mid, src.size().width, src.size().height, bitmap_config);

    AndroidBitmapInfo  info;
    void*              pixels = 0;

    try {
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC3 || src.type() == CV_8UC4);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        if(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888){
            Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if(src.type() == CV_8UC1){
                cvtColor(src, tmp, CV_GRAY2RGBA);
            }else if(src.type() == CV_8UC3){
                cvtColor(src, tmp, CV_RGB2RGBA);
            }else if(src.type() == CV_8UC4){
                if(needPremultiplyAlpha){
                    cvtColor(src, tmp, COLOR_RGBA2mRGBA);
                }else{
                    src.copyTo(tmp);
                }
            }
        }else{
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            if(src.type() == CV_8UC1){
                cvtColor(src, tmp, CV_GRAY2BGR565);
            }else if(src.type() == CV_8UC3){
                cvtColor(src, tmp, CV_RGB2BGR565);
            }else if(src.type() == CV_8UC4){
                cvtColor(src, tmp, CV_RGBA2BGR565);
            }
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return bitmap;
    }catch(cv::Exception e){
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGE("nMatToBitmap catched cv::Exception: %s", e.what());
        jclass je = env->FindClass("org/opencv/core/CvException");
        if(!je) je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return bitmap;
    }catch (...){
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGE("nMatToBitmap catched unknown exception (...)");
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nMatToBitmap}");
        return bitmap;
    }

}

void bitmap_to_mat(JNIEnv * env, jobject bitmap, Mat & dst, bool needUnPremultiplyAlpha){
    AndroidBitmapInfo  info;
    void*              pixels = 0;

    try {
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        dst.create(info.height, info.width, CV_8UC4);
        if(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888){
            Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if(needUnPremultiplyAlpha){
                cvtColor(tmp, dst, COLOR_mRGBA2RGBA);
            }else{
                tmp.copyTo(dst);
            }
        }else{
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            Mat tmp(info.height, info.width, CV_8UC2, pixels);
            cvtColor(tmp, dst, CV_BGR5652RGBA);
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    }catch(cv::Exception e){
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGE("nBitmapToMat catched cv::Exception: %s", e.what());
        jclass je = env->FindClass("org/opencv/core/CvException");
        if(!je) je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    }catch (...){
        AndroidBitmap_unlockPixels(env, bitmap);
        LOGE("nBitmapToMat catched unknown exception (...)");
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nBitmapToMat}");
        return;
    }
}

void stitch(JNIEnv *env, jobject instance, jlongArray imgsArr, long panoAddr)
{
    LOGE("test");

    Mat &pano = *(Mat *)panoAddr;

    vector<Mat> images;

    jsize len = env->GetArrayLength(imgsArr);
    jlong *imageData = env->GetLongArrayElements(imgsArr, 0);

    for(int i=0; i<len; i++){
        Mat &tMat = *(Mat *)imageData[i];
        cvtColor(tMat, tMat, CV_RGBA2RGB);
        images.push_back(tMat);
    }

    Stitcher stitcher = Stitcher::createDefault(false);
    Stitcher::Status status = stitcher.stitch(images, pano);

    if (status != Stitcher::OK)
    {
        LOGE("Stitch Faild");
        return;
    }

    return;
}
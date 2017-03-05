//
// Created by sirui on 11/17/16.
//

#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>
#include "CMT.h"
#include "stitcher.h"

using namespace std;
using namespace cv;
using namespace cmt;

extern "C" {

bool CMTinitiated = false;
SRTrack *cmtDetector = NULL;

int gWidth = 100;
int gHeight = 100;

long rect[4];

#define JNIREG_CLASS "com/example/wrapper/tracking/MainActivity"
#define JNIREG_STITCH_CLASS "com/example/wrapper/stitch/Stitcher"

__attribute__((section (".stext"))) static JNICALL void initTracking(int width, int height)
{
    gWidth = width;
    gHeight = height;
}

__attribute__((section (".stext"))) static JNICALL void cleanup()
{
    if(cmtDetector != NULL){
        delete cmtDetector;
        cmtDetector = NULL;
    }

}

__attribute__((section (".stext"))) static JNICALL jboolean setupTacking(JNIEnv *env, jobject instance, jlong addrGray,
                                                   jlong x, jlong y, jlong width,
                                                     jlong height) {


    const char *pkgName = "com.sirui.smartTable";

    jclass clz = env->GetObjectClass(instance);
    jmethodID methodId = env->GetMethodID(clz, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    if(methodId == NULL){
        return false;
    }

    methodId = env->GetMethodID(clz, "getPackageName", "()Ljava/lang/String;");
    jstring jPkgName = (jstring)env->CallObjectMethod(instance, methodId);
    if(jPkgName == NULL){
        return false;
    }

    const char *localPkgName = env->GetStringUTFChars(jPkgName, NULL);

//    if(strcmp(pkgName, localPkgName)){
//        return false;
//    }

    if (cmtDetector!=NULL)
    {
         delete cmtDetector;
    }
    cmtDetector = new SRTrack();

    Mat &im_gray = *(Mat *) addrGray;

    Point p1(x, y);
    Point p2(x + width, y + height);

    CMTinitiated = false;
    cv::Rect rect = cv::Rect(x, y, width, height);
    cmtDetector->initialize(im_gray, rect);
    CMTinitiated = true;

    return true;
}

__attribute__((section (".stext"))) static JNICALL void processFrame(JNIEnv *env, jobject instance,
                                                        jlong addrGray) {

    if (!CMTinitiated)
        return;
    Mat &im_gray = *(Mat *) addrGray;

    cmtDetector->processFrame(im_gray);

}

__attribute__((section (".stext"))) static JNICALL jintArray getRect(JNIEnv *env, jobject instance) {

    if (!CMTinitiated)
        return NULL;

    jintArray result;
    result = env->NewIntArray(8);

    RotatedRect rect = cmtDetector->bb_rot;
    Point2f vertices[4];
    rect.points(vertices);

    jint fill[8];

    {

        fill[0] = vertices[0].x;
        fill[1] = vertices[0].y;
        fill[2] = vertices[1].x;
        fill[3] = vertices[1].y;
        fill[4] = vertices[2].x;
        fill[5] = vertices[2].y;
        fill[6] = vertices[3].x;
        fill[7] = vertices[3].y;
        env->SetIntArrayRegion(result, 0, 8, fill);
        return result;
    }

    return NULL;

}


static JNINativeMethod gMethods[] = {
        {"setupTacking", "(JJJJJ)Z", (void*)setupTacking},
        {"process", "(J)V", (void*)processFrame},
        {"getRect", "()[I", (void*)getRect},
        {"cleanup", "()V", (void*)cleanup},
        {"initTracking", "(II)V", (void *)initTracking}
};

static JNINativeMethod stitchMethods[] = {
        {"stitch", "([JJ)V", (void*)stitch}
};

static int registerNativeMethods(JNIEnv* env, const char* className,
                                 JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    int ret = env->RegisterNatives(clazz, gMethods, numMethods);
    if (ret < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static int registerNatives(JNIEnv* env)
{
    if (!registerNativeMethods(env, JNIREG_CLASS, gMethods,
                               sizeof(gMethods) / sizeof(gMethods[0])))
        return JNI_FALSE;

    if(!registerNativeMethods(env, JNIREG_STITCH_CLASS, stitchMethods,
                              sizeof(stitchMethods) / sizeof(stitchMethods[0])))
        return JNI_FALSE;

    return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    assert(env != NULL);

    if (!registerNatives(env)) {
        return -1;
    }

/* success -- return valid version number */

    result = JNI_VERSION_1_4;

    return result;
}





}
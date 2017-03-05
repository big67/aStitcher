#include <jni.h>
#include <string>
#include <opencv/cv.h>

extern "C"
jstring
Java_com_example_sirui_tracking_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {

    std::string hello = "hello";
    return env->NewStringUTF(hello.c_str());
}

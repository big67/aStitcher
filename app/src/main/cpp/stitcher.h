//
// Created by a on 16/12/13.
//

#ifndef TRACKING_STITCHER_H
#define TRACKING_STITCHER_H

#include "jni.h"
#include "opencv2/core/core.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/stitcher.hpp"

using namespace cv;

void stitch(JNIEnv *env, jobject instance, jlongArray imgsArr, long panoAddr);


#endif //TRACKING_STITCHER_H

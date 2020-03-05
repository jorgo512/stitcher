#include <jni.h>
#include <string>
#include <opencv2/core/hal/interface.h>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <iostream>
using namespace cv;
using namespace xfeatures2d;
using namespace std;

//extern "C" JNIEXPORT jstring JNICALL
//Java_idg_ezio_MainActivity_stringFromJNI(
//        JNIEnv* env,
//        jobject /* this */) {
//    std::string hello = "Hello from C++";
//    return env->NewStringUTF(hello.c_str());
//}

extern "C" JNIEXPORT jintArray JNICALL
Java_idg_ezio_MainActivity_Bitmap2Grey(JNIEnv *env, jobject, jintArray buf, int w, int h){
    jint *pixels = env->GetIntArrayElements(buf, NULL);
    if(pixels == NULL){
        return NULL;
    }
    Mat img;
    cv::Mat imgData(h, w,CV_8UC4, pixels);
    cvtColor(imgData, img, CV_RGBA2GRAY);
    cvtColor(img, imgData, CV_GRAY2RGBA);
    int size = w * h;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, pixels);
    env->ReleaseIntArrayElements(buf, pixels, 0);
    return result;
}

extern "C" JNIEXPORT jintArray JNICALL
Java_idg_ezio_MainActivity_StitchImages(JNIEnv *env, jobject, jintArray fira, jintArray seca, int w, int h) {
    jint *pFir = env->GetIntArrayElements(fira, NULL);
    if (pFir == NULL) { return NULL; }
    jint *pSec = env->GetIntArrayElements(seca, NULL);
    if (pSec == NULL) { return NULL; }

    int start, size = w * h;
    Mat fir, sec;

    cv::Mat imgFir(h, w, CV_8UC4, pFir);
    cvtColor(imgFir, fir, CV_RGBA2GRAY);
    cv::Mat imgSec(h, w, CV_8UC4, pSec);
    cvtColor(imgSec, sec, CV_RGBA2GRAY);


//    w = 2048;
//    h = 1280;
//    size = w* h;
//    resize(fir, fir, Size(w, h));
//    resize(sec, sec, Size(w, h));

    Ptr<SURF> Detector = SURF::create(2000.0f);
    vector<KeyPoint> keyPoint1, keyPoint2;
    Mat imageDesc1, imageDesc2;
    Detector->detectAndCompute(fir, noArray(), keyPoint1, imageDesc1);
    Detector->detectAndCompute(sec, noArray(), keyPoint2, imageDesc2);

    FlannBasedMatcher matcher;
    vector<vector<DMatch> > matchePoints;
    vector<Mat> train_desc(1, imageDesc2);
    matcher.add(train_desc);
    matcher.train();
    matcher.knnMatch(imageDesc1, matchePoints, 2);
    vector<DMatch> GoodMatchePoints;
    for (int i = 0; i < matchePoints.size(); i++) {
        if (matchePoints[i][0].distance < 0.4 * matchePoints[i][1].distance) {
            GoodMatchePoints.push_back(matchePoints[i][0]);
        }
    }
    vector<Point2f> imagePoints1, imagePoints2;
    for (int i = 0; i < GoodMatchePoints.size(); i++) {
        imagePoints1.push_back(keyPoint1[GoodMatchePoints[i].queryIdx].pt);
        imagePoints2.push_back(keyPoint2[GoodMatchePoints[i].trainIdx].pt);
    }

    Mat homo = findHomography(imagePoints2, imagePoints1, CV_RANSAC);
    Mat lt = homo * (Mat_<double>(3, 1) << 0, 0, 1);                //Point2f(lt.at<double>(0) / lt.at<double>(2), lt.at<double>(1) / lt.at<double>(2));
    Mat lb = homo * (Mat_<double>(3, 1) << 0, sec.rows, 1);         //Point2f(lb.at<double>(0) / lb.at<double>(2), lb.at<double>(1) / lb.at<double>(2));
    Mat rt = homo * (Mat_<double>(3, 1) << sec.cols, 0, 1);         //Point2f(rt.at<double>(0) / rt.at<double>(2), rt.at<double>(1) / rt.at<double>(2));
    Mat rb = homo * (Mat_<double>(3, 1) << sec.cols, sec.rows, 1);  //Point2f(rb.at<double>(0) / rb.at<double>(2), rb.at<double>(1) / rb.at<double>(2));

    start = int(MIN(lb.at<double>(0) / lb.at<double>(2), lt.at<double>(0) / lt.at<double>(2)));
    w = int(MAX(rt.at<double>(0) / rt.at<double>(2), rb.at<double>(0) / rb.at<double>(2)));
    h = sec.rows;
    size = w* h;

    Mat transform;
    warpPerspective(sec, transform, homo, Size(w, h));

    int *buf = new int [size];
    int i, j, val = 0;
    float weight = fir.cols - start;
    float alpha;
    for (j = 0; j < h; j++) {
        int *p = buf+j*w;
        uchar *l = fir.ptr<uchar>(j);
        for (i = 0; i < start; i++) {
            val = *l++;
            *p++ = 0xFF000000|val|val<<8|val<<16;
        }
        uchar *t = transform.ptr<uchar>(j)+ start;
        for (i = start; i < fir.cols; i++) {
            alpha = (fir.cols - i) / weight;
            val = int(*l++ * alpha + *t++ * (1 - alpha));
            *p++ = 0xFF000000|val|val<<8|val<<16;
        }
        for (i = fir.cols; i < w; i++) {
            val = *t++;
            *p++ = 0xFF000000|val|val<<8|val<<16;
        }
    }
    buf[0] = w;
    buf[1] = h;

    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, buf);
    delete [] buf;

    env->ReleaseIntArrayElements(seca, pSec, 0);
    env->ReleaseIntArrayElements(fira, pFir, 0);

    return result;
}
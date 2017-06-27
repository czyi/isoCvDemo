//
//  cvUtiil.hpp
//  cvdemo
//
//  Created by Ziyi Chen on 6/8/17.
//  Copyright © 2017 Ziyi Chen. All rights reserved.
//

#ifndef cvUtiil_hpp
#define cvUtiil_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>


#include <opencv2/opencv.hpp>
#import <opencv2/imgproc/types_c.h>
#import <opencv2/imgcodecs/ios.h>
#import <opencv2/highgui.hpp>
#import <opencv2/highgui/highgui.hpp>
#import <opencv2/highgui/highgui_c.h>
#include <opencv2/videoio/cap_ios.h>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

using namespace cv;
using namespace std;

class cvUtil{
    Mat img0; //initial image
    Mat img1; //previous / first image
    vector<Mat> initialImgBuf;
    double blurThreshold, blurScale;
    int countBlurSkip, countFromInitial, CountFromPrevious;
    
    Ptr<Feature2D> b;
    vector<KeyPoint> keyImg0, keyImg1; //key point of img
    Mat descImg0, descImg1; //descriptor of img
    
    Mat homo0, preHomo0, preHomo, preHomo2;
    vector<Point2d> iniPts, prePts, curPts;
    
    int sepLeft, sepRight, sepTop, sepButtom;
    int left, right, top, buttom;
    int width, height;
    
    Mat nullHomo;
    vector<Mat> homoList;
    int listpt;
    int featurePointNum = 30;
    
    
    Mat curGray, preGray;
    vector<Point2f> preOptPts, curOptPts;
    vector<Point2f> initial;
    vector<Point2f> features; // 检测的特征
    int maxCount = 500; // 检测的最大特征数
    double qLevel = 0.01; // 特征检测的等级
    double minDist = 10.0; // 两特征点之间的最小距离
    vector<uchar> status; // 跟踪特征的状态，特征的流发现为1，否则为0
    vector<float> err;
    
public:
    cvUtil();
    ~cvUtil();
    bool setInitialPts(int left, int right, int top, int buttom, int w, int h);
    bool initialImageFeature(Mat img1);
    Mat homoMatrixToInitial(Mat img2);
    Mat homoMatrixToPrevious(Mat img2);
    Mat homoMatrixCombine(Mat img2, int flag);
    Mat filterHomo(Mat img2);
    double homoDiffSum(Mat homo1, Mat homo2);
    double imageDiff(Mat img1, Mat img2);
    int tracking(Mat curframe);
    bool addNewPoints();
    bool acceptTrackedPoint(int i);
    Mat grabCut(Mat img);
    bool reset();
};



#endif /* cvUtiil_hpp */

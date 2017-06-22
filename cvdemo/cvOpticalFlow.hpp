//
//  cvOpticalFlow.hpp
//  cvdemo
//
//  Created by Ziyi Chen on 6/21/17.
//  Copyright © 2017 Ziyi Chen. All rights reserved.
//

#ifndef cvOpticalFlow_hpp
#define cvOpticalFlow_hpp

#include <stdio.h>
#include "cvUtiil.hpp"

//#include <stdio.h>
//#include <iostream>
//#include <vector>
//#include <string>
//#include <unistd.h>
//
//
//#include <opencv2/opencv.hpp>
//#import <opencv2/imgproc/types_c.h>
//#import <opencv2/imgcodecs/ios.h>
//#import <opencv2/highgui.hpp>
//#import <opencv2/highgui/highgui.hpp>
//#import <opencv2/highgui/highgui_c.h>
//#include <opencv2/videoio/cap_ios.h>
//
//#include "opencv2/imgcodecs.hpp"
//#include "opencv2/imgproc.hpp"

using namespace cv;
using namespace std;


class cvOpticalFlow{
    Mat img0; //initial image
    Mat img1; //previous / first image
    
    string window_name = "optical flow tracking";
    Mat curGray, preGray;

    vector<Point2f> prePts, curPts;
    vector<Point2f> initial;
    vector<Point2f> features; // 检测的特征
    int maxCount = 500; // 检测的最大特征数
    double qLevel = 0.01; // 特征检测的等级
    double minDist = 10.0; // 两特征点之间的最小距离
    vector<uchar> status; // 跟踪特征的状态，特征的流发现为1，否则为0
    vector<float> err;
    
public:
    cvOpticalFlow();
    ~cvOpticalFlow();
    
    Mat tracking(Mat curframe);
    bool addNewPoints();
    bool acceptTrackedPoint(int i);
};






#endif /* cvOpticalFlow_hpp */

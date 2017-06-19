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


using namespace cv;
using namespace std;

class cvUtil{
    Mat img0; //initial image
    Mat img1; //previous / first image
    
    Ptr<Feature2D> b;
    vector<KeyPoint> keyImg0, keyImg1; //key point of img
    Mat descImg0, descImg1; //descriptor of img
    
    Mat homo0, preHomo0, preHomo;
    vector<Point2d> iniPts, prePts, curPts;
    
    int left, right, top, buttom;
    int width, height;
    
    vector<Mat> homolist;
    int listpt;
    
public:
    cvUtil();
    ~cvUtil();
    bool setInitialPts(int left, int right, int top, int buttom, int w, int h);
    bool initialImageFeature(Mat img1);
    Mat homoMatrixToInitial(Mat img2, CvPoint upLeft, CvPoint lowRight);
    Mat homoMatrixToPrevious(Mat img2);
    Mat homoMatrixCombine(Mat img2, int flag);
    double homoDiffSum(Mat homo1, Mat homo2);
    bool reset();
};



#endif /* cvUtiil_hpp */

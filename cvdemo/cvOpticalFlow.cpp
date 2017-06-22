//
//  cvOpticalFlow.cpp
//  cvdemo
//
//  Created by Ziyi Chen on 6/21/17.
//  Copyright © 2017 Ziyi Chen. All rights reserved.
//

#include "cvOpticalFlow.hpp"

using namespace std;
using namespace cv;

cvOpticalFlow::cvOpticalFlow(){
    
}

cvOpticalFlow::~cvOpticalFlow(){
    
}


Mat cvOpticalFlow::tracking(Mat curframe)
{
    if(curframe.rows*curframe.cols <=0 ){
        cout << "no frame" << endl;
        return Mat();
    }
    cvtColor(curframe, curGray, COLOR_BGR2GRAY);
    
    Mat output;
    curframe.copyTo(output);
    
    // 添加特征点
    if (addNewPoints())
    {
        goodFeaturesToTrack(curGray, features, maxCount, qLevel, minDist);
        prePts.insert(prePts.end(), features.begin(), features.end());
        initial.insert(initial.end(), features.begin(), features.end());
    }
    
    
    if (preGray.empty())
    {
        curGray.copyTo(preGray);
    }
    
    //clock_t startTime=clock();
    // l-k光流法运动估计
    calcOpticalFlowPyrLK(preGray, curGray, prePts, curPts, status, err);
    // 去掉一些不好的特征点
    int k = 0;
    for (size_t i=0; i<curPts.size(); i++)
    {
        if (acceptTrackedPoint(i))
        {
            initial[k] = initial[i];
            curPts[k++] = curPts[i];
        }
    }
    curPts.resize(k);
    initial.resize(k);
    //clock_t endTime=clock();
    //cout << "calcOpticalFlowPyrLK time is : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    
    // 显示特征点和运动轨迹
    for (size_t i=0; i<curPts.size(); i++)
    {
//        line(output, initial[i], curPts[i], Scalar(0, 0, 255));
//        circle(output, curPts[i], 3, Scalar(0, 255, 0), -1);
        line(curframe, initial[i], curPts[i], Scalar(0, 0, 255));
        circle(curframe, curPts[i], 3, Scalar(0, 255, 0), -1);
    }
    
    // 把当前跟踪结果作为下一此参考
    swap(curPts, prePts);
    swap(curGray, preGray);
    
    return curframe;
}


//-------------------------------------------------------------------------------------------------
// function: addNewPoints
// brief: 检测新点是否应该被添加
// parameter:
// return: 是否被添加标志
//-------------------------------------------------------------------------------------------------
bool cvOpticalFlow::addNewPoints()
{
    return prePts.size() <= 10;
}


//-------------------------------------------------------------------------------------------------
// function: acceptTrackedPoint
// brief: 决定哪些跟踪点被接受
// parameter:
// return:
//-------------------------------------------------------------------------------------------------
bool cvOpticalFlow::acceptTrackedPoint(int i)
{
    return status[i] && ((abs(prePts[i].x - curPts[i].x) + abs(prePts[i].y - curPts[i].y)) > 2);
}

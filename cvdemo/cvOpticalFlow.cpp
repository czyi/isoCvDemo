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
    preHomo = Mat(3,3, CV_64F);
    preHomo.at<double>(2,2)=-10;
}

cvOpticalFlow::~cvOpticalFlow(){
    
}

bool cvOpticalFlow::setInitialPts(int l, int r, int t, int b, int w, int h){
    left=l;
    right=r;
    top=t;
    buttom=b;
    
    width=w;
    height=h;
    
    return true;
}

bool cvOpticalFlow::initialFeaturePoint(Mat curframe){
    if(curframe.rows*curframe.cols <=0 ){
        cout << "no frame" << endl;
        return false;
    }
    cvtColor(curframe, curGray, COLOR_BGR2GRAY);
    
    Mat output;
    curframe.copyTo(output);
    
    // 添加特征点
    if (addNewPoints())
    {
        goodFeaturesToTrack(curGray, features, maxCount, qLevel, minDist);
        preOptPts.insert(preOptPts.end(), features.begin(), features.end());
        initial.insert(initial.end(), features.begin(), features.end());
    }
    
    if (preGray.empty()) curGray.copyTo(preGray);
    
    calcOpticalFlowPyrLK(preGray, curGray, preOptPts, curOptPts, status, err);
    int k = 0;
    for (int i=0; i<curOptPts.size(); i++)
    {
        if (acceptTrackedPoint(i)){
            initial[k] = initial[i];
            curOptPts[k++] = curOptPts[i];
        }
    }
    curOptPts.resize(k);
    initial.resize(k);
    
    // 显示特征点和运动轨迹
    for (size_t i=0; i<curOptPts.size(); i++){
        line(curframe, initial[i], curOptPts[i], Scalar(0, 0, 255));
        circle(curframe, curOptPts[i], 3, Scalar(0, 255, 0), -1);
    }
    
    // 把当前跟踪结果作为下一此参考
    swap(curOptPts, preOptPts);
    swap(curGray, preGray);
    
    if(k>=30) return true;
    return false;
}

Mat cvOpticalFlow::tracking(Mat curframe){
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
        preOptPts.insert(preOptPts.end(), features.begin(), features.end());
        initial.insert(initial.end(), features.begin(), features.end());
    }
    
    if (preGray.empty()) curGray.copyTo(preGray);
    
    double borderLeft=width, borderRight=0, borderTop=height, borderButtom=0;
    for(int i=0; i<preOptPts.size(); i++){
        if(borderLeft>preOptPts[i].x) borderLeft=preOptPts[i].x;
        if(borderRight<preOptPts[i].x) borderRight=preOptPts[i].x;
        if(borderTop>preOptPts[i].y) borderTop=preOptPts[i].y;
        if(borderButtom<preOptPts[i].y) borderLeft=preOptPts[i].y;
    }
    
    calcOpticalFlowPyrLK(preGray, curGray, preOptPts, curOptPts, status, err);
    int k = 0;
    for (int i=0; i<curOptPts.size(); i++)
    {
        //if(curOptPts[i].x<borderLeft || curOptPts[i].x>borderRight || curOptPts[i].y<borderTop || curOptPts[i].y>borderTop) continue;
        
        if (acceptTrackedPoint(i)){
            initial[k] = initial[i];
            curOptPts[k++] = curOptPts[i];
        }
    }
    curOptPts.resize(k);
    initial.resize(k);
    
    // 显示特征点和运动轨迹
    for (size_t i=0; i<curOptPts.size(); i++){
        line(curframe, initial[i], curOptPts[i], Scalar(0, 0, 255));
        circle(curframe, curOptPts[i], 3, Scalar(0, 255, 0), -1);
        
        //if(curOptPts[i].x<borderLeft || curOptPts[i].x>borderRight || curOptPts[i].y<borderTop || curOptPts[i].y>borderTop)
        if(curOptPts[i].x<left || curOptPts[i].x>right || curOptPts[i].y<top || curOptPts[i].y>buttom){
            circle(curframe, initial[i], 3, Scalar(0, 0, 0), -1);
        }
        else{
            circle(curframe, initial[i], 3, Scalar(255, 0, 0), -1);
        }
    }
    
    // 把当前跟踪结果作为下一此参考
    swap(curOptPts, preOptPts);
    swap(curGray, preGray);
    
    return curframe;
}

Mat cvOpticalFlow::trackingAndSeperate(Mat curframe){
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
        preOptPts.insert(preOptPts.end(), features.begin(), features.end());
        initial.insert(initial.end(), features.begin(), features.end());
    }
    
    if (preGray.empty()) curGray.copyTo(preGray);
    
    calcOpticalFlowPyrLK(preGray, curGray, preOptPts, curOptPts, status, err);
    
    vector<Point2f> remainPts;
    int k = 0;
    for (int i=0; i<curOptPts.size(); i++)
    {
        if (acceptTrackedPoint(i)){
            initial[k] = initial[i];
            curOptPts[k++] = curOptPts[i];
            remainPts.push_back(preOptPts[i]);
        }
    }
    curOptPts.resize(k);
    initial.resize(k);
    
//    cout << "curOptPts size is " << curOptPts.size() << endl;
//    cout << "remain pts size is " << remainPts.size() << endl;
    
    cout << "cur pts size is " << curOptPts.size() << endl;
    if(curOptPts.size()==0){
        cout << "no point" << endl;
        circle(curframe, cvPoint(20,20), 10, Scalar(0, 255, 0), -1);
    }
    
    //need at least 8 points to calculate homo matrix
    if(k>=9){
        vector<Point2f> expPts(k);
        Mat homoMatrix=findHomography(remainPts,curOptPts,CV_RANSAC);
        perspectiveTransform(remainPts, expPts, homoMatrix);
        
        double threshold=0;
        for(size_t i=0; i<curOptPts.size(); i++){
            threshold+=abs(expPts[i].x-curOptPts[i].x)+abs(expPts[i].y-curOptPts[i].y);
        }
        threshold/=k;
        
        //cout << curOptPts.size() << ", " << k << endl;
        //cout << "thershold is " << threshold*2 << endl;
        
        // 显示特征点和运动轨迹
        for (size_t i=0; i<curOptPts.size(); i++){
            line(curframe, initial[i], curOptPts[i], Scalar(0, 0, 255));
            //circle(curframe, curOptPts[i], 3, Scalar(0, 255, 0), -1);
            
            if(abs(expPts[i].x-curOptPts[i].x)+abs(expPts[i].y-curOptPts[i].y)>threshold*2){
                circle(curframe, curOptPts[i], 3, Scalar(0, 255, 0), -1);
            }
        }
    }
    
    
    // 把当前跟踪结果作为下一此参考
    swap(curOptPts, preOptPts);
    swap(curGray, preGray);
    
    return curframe;
}

Mat cvOpticalFlow::trackingAndSeperateAndMatrix(Mat curframe){
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
        preOptPts.insert(preOptPts.end(), features.begin(), features.end());
        initial.insert(initial.end(), features.begin(), features.end());
    }
    
    if (preGray.empty()) curGray.copyTo(preGray);
    
    calcOpticalFlowPyrLK(preGray, curGray, preOptPts, curOptPts, status, err);
    
    vector<Point2f> remainPts;
    int k = 0;
    for (int i=0; i<curOptPts.size(); i++)
    {
        if (acceptTrackedPoint(i)){
            initial[k] = initial[i];
            curOptPts[k++] = curOptPts[i];
            remainPts.push_back(preOptPts[i]);
        }
    }
    curOptPts.resize(k);
    initial.resize(k);
    
    //    cout << "curOptPts size is " << curOptPts.size() << endl;
    //    cout << "remain pts size is " << remainPts.size() << endl;
    
    //need at least 4 points to calculate homo matrix
    if(k>=5){
        vector<Point2f> expPts(k), preFrontPts, curFrontPts;
        Mat homoMatrix=findHomography(remainPts,curOptPts,CV_RANSAC);
        perspectiveTransform(remainPts, expPts, homoMatrix);
        
        double threshold=0;
        for(size_t i=0; i<curOptPts.size(); i++){
            threshold+=abs(expPts[i].x-curOptPts[i].x)+abs(expPts[i].y-curOptPts[i].y);
        }
        threshold/=k;
        
        //cout << curOptPts.size() << ", " << k << endl;
        cout << "thershold is " << threshold*2 << endl;
        
        // 显示特征点和运动轨迹
        for (size_t i=0; i<curOptPts.size(); i++){
            line(curframe, initial[i], curOptPts[i], Scalar(0, 0, 255));
            //circle(curframe, curOptPts[i], 3, Scalar(0, 255, 0), -1);
            
            if(abs(expPts[i].x-curOptPts[i].x)+abs(expPts[i].y-curOptPts[i].y)>threshold*2){
                circle(curframe, curOptPts[i], 3, Scalar(0, 255, 0), -1);
                preFrontPts.push_back(initial[i]);
                curFrontPts.push_back(curOptPts[i]);
            }
        }
        
        if(preFrontPts.size()>=5){
            Mat curHomo=findHomography(preFrontPts,curFrontPts,CV_RANSAC);
            if(preHomo.cols<3 || preHomo.rows<3 || preHomo.at<double>(2,2)<0){
                preHomo=curHomo;
            }
            else if(curHomo.rows==3 && curHomo.cols==3){
                preHomo=preHomo*curHomo;
            }
        }
    }
    
    // 把当前跟踪结果作为下一此参考
    swap(curOptPts, preOptPts);
    swap(curGray, preGray);
    
    return preHomo;
}

bool cvOpticalFlow::addNewPoints(){
    return preOptPts.size() <= 30;
}

bool cvOpticalFlow::acceptTrackedPoint(int i){
    return status[i] && ((abs(preOptPts[i].x - curOptPts[i].x) + abs(preOptPts[i].y - curOptPts[i].y)) > 2);
}

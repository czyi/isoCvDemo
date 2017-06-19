//
//  cvUtiil.cpp
//  cvdemo
//
//  Created by Ziyi Chen on 6/8/17.
//  Copyright © 2017 Ziyi Chen. All rights reserved.
//

#include "cvUtiil.hpp"

using namespace std;
using namespace cv;



cvUtil::cvUtil(){
    b = ORB::create();
    prePts.reserve(4);
    curPts.reserve(4);
    iniPts.reserve(4);
    homolist.reserve(10);
    listpt = -10;
}

cvUtil::~cvUtil(){
    
}

bool cvUtil::setInitialPts(int left, int right, int top, int buttom, int w, int h){
    this->left=left;
    this->right=right;
    this->top=top;
    this->buttom=buttom;
    
    iniPts.push_back(cvPoint(left+50, top+30));
    iniPts.push_back(cvPoint(left+50, buttom-30));
    iniPts.push_back(cvPoint(right-50, buttom-30));
    iniPts.push_back(cvPoint(right-50, top+30));
    
    prePts.push_back(cvPoint(left+50, top+30));
    prePts.push_back(cvPoint(left+50, buttom-30));
    prePts.push_back(cvPoint(right-50, buttom-30));
    prePts.push_back(cvPoint(right-50, top+30));
    
    curPts.push_back(cvPoint(left+50, top+30));
    curPts.push_back(cvPoint(left+50, buttom-30));
    curPts.push_back(cvPoint(right-50, buttom-30));
    curPts.push_back(cvPoint(right-50, top+30));
    
    width=w;
    height=h;
    
    return true;
}

bool cvUtil::initialImageFeature(Mat img0){
    this->img0=img0;
    clock_t startTime, endTime;
    
    if(img0.rows*img0.cols<=0)
    {
        cout << "no img0" << endl;
        return false;
    }
//    
//    Mat mask;
//    grabCut(img0, mask, Rect( 190, 255, 145, 245 ), Mat(), Mat(), 0, GC_INIT_WITH_RECT);
    
    startTime=clock();
    b->detectAndCompute(img0, Mat(), keyImg0, descImg0, false);
    endTime = clock();
    cout << "first image feature detect time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    
    return true;
}

cv::Mat cvUtil::homoMatrixToInitial(Mat img2, CvPoint upLeft, CvPoint lowRight){
    clock_t startTime, endTime;
    
    if(img0.rows*img0.cols<=0)
    {
        cout << "no img0" << endl;
        //return NULL;
    }
    if(img2.rows*img2.cols<=0)
    {
        cout << "no img2" << endl;
        //return NULL;
    }
    
    //Ptr<Feature2D> b;
    
    Ptr<DescriptorMatcher> descriptorMatcher;
    vector<DMatch> matches;
    vector<KeyPoint> keyImg2;
    Mat descImg2;
    
    startTime=clock();
    b->detectAndCompute(img2, Mat(),keyImg2, descImg2, false);
    endTime = clock();
    cout << "second image feature detect time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    startTime=clock();
    descriptorMatcher = DescriptorMatcher::create("BruteForce-Hamming");
    descriptorMatcher->match(descImg0, descImg2, matches, Mat());
    endTime = clock();
    cout << "feature matching time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    
    startTime=clock();
    Mat index;
    int nbMatch=int(matches.size());
    Mat tab(nbMatch, 1, CV_32F);
    for (int i = 0; i<nbMatch; i++)
    {
        tab.at<float>(i, 0) = matches[i].distance;
    }
    sortIdx(tab, index, SORT_EVERY_COLUMN + SORT_ASCENDING);
    vector<DMatch> bestMatches;
    vector<Point2f> imagePoints0, imagePoints2;
    
    for (int i = 0; i<nbMatch; i++)
    {
        
        KeyPoint kp=keyImg0[index.at<int>(i, 0)];
        if(kp.pt.x < upLeft.x || kp.pt.x > lowRight.x || kp.pt.y < upLeft.y || kp.pt.y >lowRight.y) continue;
        
        DMatch current=matches[index.at<int>(i, 0)];
        bestMatches.push_back(current);
        
        imagePoints0.push_back(keyImg0[current.queryIdx].pt);
        imagePoints2.push_back(keyImg2[current.trainIdx].pt);
        
        if(bestMatches.size()>=30) break;
        //if(imagePoints1.size()>=30) break;
    }
    
    preHomo0=homo0;
    homo0=findHomography(imagePoints0,imagePoints2,CV_RANSAC);
    cout<<"Transform Matrix：\n"<<homo0<<endl<<endl;
    endTime = clock();
    cout << "homo matrix calculate time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

//    Mat result;
//    rectangle(img1, upLeft, lowRight, cvScalar(1), 3);
//    drawMatches(img1, keyImg1, img2, keyImg2, bestMatches, result);

    return homo0;
}

cv::Mat cvUtil::homoMatrixToPrevious(Mat img2){
    clock_t startTime, endTime;
    int flag=1;
    
    if(img1.rows*img1.cols<=0)
    {
        if(img0.rows*img0.cols<=0)
        {
            cout << "no img1" << endl;
            //return NULL;
        }
        else{
            img1=img0;
            keyImg1=keyImg0;
            descImg1=descImg0;
            flag=0;
        }
    }
    if(img2.rows*img2.cols<=0)
    {
        cout << "no img2" << endl;
    }

    Ptr<DescriptorMatcher> descriptorMatcher;
    vector<DMatch> matches;
    vector<KeyPoint> keyImg2;
    Mat descImg2;
    
    startTime=clock();
    b->detectAndCompute(img2, Mat(),keyImg2, descImg2, false);
    endTime = clock();
    cout << "second image feature detect time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    startTime=clock();
    descriptorMatcher = DescriptorMatcher::create("BruteForce-Hamming");
    descriptorMatcher->match(descImg1, descImg2, matches, Mat());
    endTime = clock();
    cout << "feature matching time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    
    startTime=clock();
    Mat index;
    int nbMatch=int(matches.size());
    Mat tab(nbMatch, 1, CV_32F);
    for (int i = 0; i<nbMatch; i++)
    {
        tab.at<float>(i, 0) = matches[i].distance;
    }
    sortIdx(tab, index, SORT_EVERY_COLUMN + SORT_ASCENDING);
    vector<DMatch> bestMatches;
    vector<Point2f> imagePoints1, imagePoints2;
    
    for (int i = 0; i<nbMatch; i++)
    {
        
        KeyPoint kp=keyImg1[index.at<int>(i, 0)];
        //if(kp.pt.x < upLeft.x || kp.pt.x > lowRight.x || kp.pt.y < upLeft.y || kp.pt.y >lowRight.y) continue;
        if((kp.pt.x < prePts[0].x && kp.pt.x < prePts[1].x) || (kp.pt.x > prePts[2].x && kp.pt.x > prePts[3].x) || (kp.pt.y < prePts[0].y && kp.pt.y < prePts[3].y) || (kp.pt.y >prePts[2].y && kp.pt.y >prePts[1].y)) continue;
        
        DMatch current=matches[index.at<int>(i, 0)];
        bestMatches.push_back(current);
        
        imagePoints1.push_back(keyImg1[current.queryIdx].pt);
        imagePoints2.push_back(keyImg2[current.trainIdx].pt);
        
        if(bestMatches.size()>=30) break;
        //if(imagePoints1.size()>=30) break;
    }
    
    Mat homo=Mat(3,3, CV_64F);
    try{
        homo=findHomography(imagePoints1,imagePoints2,CV_RANSAC);
    }
    catch (Exception& e){
        homo.at<double>(2,2)=-1;
        return homo;
    }
    
    cout<<"Transform Matrix：\n"<<homo<<endl<<endl;
    endTime = clock();
    cout << "homo matrix calculate time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    
//    cv::Point points[1][4];
    //perspectiveTransform(prePts, curPts, homo);
    
    if(flag)
    {
        homo=preHomo*homo;
    }
    perspectiveTransform(iniPts, curPts, homo);
    
//    for(int i=0; i<4; i++)
//    {
//        points[0][i]=curPts[i];
//    }
//    
    img1=img2;
    keyImg1=keyImg2;
    descImg1=descImg2;
    prePts=curPts;
    preHomo=homo;
    
    return homo;
    
//    const cv::Point* pt[1] = { points[0] };
//    int npt[1] = {4};
//    polylines(img2, pt, npt, 1, 1, Scalar(1),3);
//    return img2;
}

Mat cvUtil::homoMatrixCombine(Mat img2, int flag){
    Mat homo1=homoMatrixToInitial(img2, cvPoint(left, top), cvPoint(right, buttom));
    Mat homo2=homoMatrixToPrevious(img2);
    
//    cout << "homo 1 : " << "\n" << homo1 << "\n" << endl;
//    cout << "homo 2 : " << "\n" << homo2 << "\n" << endl;
//    cout << "diff : " << "\n" << diff << "\n" << endl;
    
    double sum=homoDiffSum(homo1, homo2);
    cout << "sum is " << sum << endl;
 
    Mat homo;
    
    if(flag==1){
        if(homo2.at<double>(2,2)<0 || sum>0.5){
            preHomo=homo1;
            homo = homo1;
        }
        else{
            homo = (homo1 + homo2)/2;
        }
    }
    else if(flag==2){
        if(homo2.at<double>(2,2)<0 || sum<1){
            preHomo=homo1;
            homo = homo1;
        }
        else{
            //homo = (homo1 + homo2)/2;
            homo=homo2;
        }
    }
    
    return homo;
    
    
//    Mat sumhomo=Mat(3,3,CV_32F);
//    for(int i=0; i<3; i++){
//        for(int j=0; j<3; j++){
//            sumhomo.at<double>(i,j)=0;
//        }
//    }
//    cout << "sum homo is " << "\n" << sumhomo << endl;
//
//    if(listpt<0){
//        homolist.push_back(homo);
//        listpt++;
//        
//        int i=0;
//        for(; i<listpt+9; i++){
//            sumhomo = sumhomo+homo;
//        }
//        
//        cout << "avg homo is " << "\n" << sumhomo/i << endl;
//        return sumhomo/i;
//    }
//
//    if(listpt>=10) listpt %= 10;
//    homolist[listpt]=homo;
//    
//    for(int i=0; i<10; i++)
//    {
//        sumhomo+=homolist[i];
//    }
//    
//    cout << "avg homo is " << "\n" << sumhomo/10 << endl;
//    return sumhomo/10;
    
//    if(homo2.at<double>(2,2)<0){
//        preHomo=homo1;
//        return homo1;        
//    }
//    
//    if(sum<0.5){
//        return (homo1 + homo2)/2;
//    }
//    
//    if(homoDiffSum(homo2, preHomo)<homoDiffSum(homo1, preHomo0)){
//        return 0.8*homo2+0.2*homo1;
//    }
//
//    return 0.8*homo1+0.2*homo2;

}

double cvUtil::homoDiffSum(Mat homo1, Mat homo2){
    if(homo1.rows<2 || homo1.cols <2 || homo2.rows<2 || homo2.cols <2){
        return 0;
    }
    
    Mat diff = homo1-homo2;
    
    diff.at<double>(0,2) /= width;
    diff.at<double>(1,2) /= height;
    
    double sum=0;
    for(int i=0; i<3; i++){
        for(int j=0; j<3; j++){
//            sum += diff.at<double>(i,j) * diff.at<double>(i,j);
            sum += abs(diff.at<double>(i,j));
        }
    }
    return sum;
}

bool cvUtil::reset(){
    
    return true;
}

//Mat tranformMatrix(Mat img1, Mat img2, CvPoint upLeft, CvPoint lowRight)
//{
//    clock_t startTime, endTime;
//
//    if(img2.rows*img2.cols<=0)
//    {
//        cout << "no img1" << endl;
//        //return NULL;
//    }
//    
//    Ptr<Feature2D> b;
//    
//    Ptr<DescriptorMatcher> descriptorMatcher;
//    vector<DMatch> matches;
//    vector<KeyPoint> keyImg1, keyImg2;
//    Mat descImg1, descImg2;
//    
//    b = ORB::create();
//    
//    startTime=clock();
//    b->detectAndCompute(img1, Mat(), keyImg1, descImg1, false);
//    endTime = clock();
//    cout << "first image feature detect time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
//    
//    startTime=clock();
//    b->detectAndCompute(img2, Mat(),keyImg2, descImg2, false);
//    endTime = clock();
//    cout << "second image feature detect time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
//    startTime=clock();
//    descriptorMatcher = DescriptorMatcher::create("BruteForce-Hamming");
//    descriptorMatcher->match(descImg1, descImg2, matches, Mat());
//    endTime = clock();
//    cout << "feature matching time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
//
//    startTime=clock();
//    Mat index;
//    int nbMatch=int(matches.size());
//    Mat tab(nbMatch, 1, CV_32F);
//    for (int i = 0; i<nbMatch; i++)
//    {
//        tab.at<float>(i, 0) = matches[i].distance;
//    }
//    sortIdx(tab, index, SORT_EVERY_COLUMN + SORT_ASCENDING);
//    vector<DMatch> bestMatches;
//    vector<Point2f> imagePoints1, imagePoints2;
//    
//    for (int i = 0; i<nbMatch; i++)
//    {
//        
//        KeyPoint kp=keyImg1[index.at<int>(i, 0)];
//        if(kp.pt.x < upLeft.x || kp.pt.x > lowRight.x || kp.pt.y < upLeft.y || kp.pt.y >lowRight.y) continue;
//        
//        DMatch current=matches[index.at<int>(i, 0)];
//        bestMatches.push_back(current);
//        
//        imagePoints1.push_back(keyImg1[current.queryIdx].pt);
//        imagePoints2.push_back(keyImg2[current.trainIdx].pt);
//        
//        if(bestMatches.size()>=30) break;
//        //if(imagePoints1.size()>=30) break;
//    }
//    
//    Mat homo=findHomography(imagePoints1,imagePoints2,CV_RANSAC);
//    cout<<"Transform Matrix：\n"<<homo<<endl<<endl;
//    endTime = clock();
//    cout << "homo matrix calculate time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
//    //    [runtimeLab setText:[runtimeLab.text stringByAppendingString:[NSString stringWithFormat:@"\nhomo matrix calculate time :  %f s",(double)(endTime - startTime) / CLOCKS_PER_SEC]]];
//    
//    Mat result;
//    rectangle(img1, upLeft, lowRight, cvScalar(1), 3);
//    drawMatches(img1, keyImg1, img2, keyImg2, bestMatches, result);
//    //    imshow("result", result);
//    //    imwrite("result.jpg",result);
//    
//    vector<Point2d> rectPts(4), transPts(4);
//    rectPts[0]=upLeft;
//    rectPts[1]=cvPoint(upLeft.x, lowRight.y);
//    rectPts[2]=lowRight;
//    rectPts[3]=cvPoint(lowRight.x, upLeft.y);
//    
//    cv::Point points[1][4];
//    perspectiveTransform(rectPts, transPts, homo);
//    for(int i=0; i<4; i++)
//    {
//        //cout << transPts[i] << endl;
//        points[0][i]=transPts[i];
//    }
//    
//    const cv::Point* pt[1] = { points[0] };
//    int npt[1] = {4};
//    //polylines(img2, pt, npt, 1, 1, Scalar(250,0,0)) ;
//    polylines(img2, pt, npt, 1, 1, Scalar(1),3);
//    //imshow("img2_poly", img2);
//    
//    Mat transResult(img1.rows,img1.cols + img2.cols+1,img1.type());
//    img1.colRange(0,img1.cols).copyTo(transResult.colRange(0,img1.cols));
//    img2.colRange(0,img2.cols).copyTo(transResult.colRange(img2.cols+1,transResult.cols));
//    //    imshow("tansResult",transResult);
//    //    imwrite("tansResult.jpg",transResult);
//    
//    //waitKey();
//    return transResult;
//}

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

static void getBinMask( const Mat& comMask, Mat& binMask )
{
    if( comMask.empty() || comMask.type()!=CV_8UC1 )
        CV_Error( Error::StsBadArg, "comMask is empty or has incorrect type (not CV_8UC1)" );
    if( binMask.empty() || binMask.rows!=comMask.rows || binMask.cols!=comMask.cols )
        binMask.create( comMask.size(), CV_8UC1 );
    binMask = comMask & 1;
}

//static double contrast_measure(vector<String>& fileName, int count)
//calculate the blurness
static double contrast_measure(Mat image)
{
    //Mat image=imread(fileName[count]);
    Mat dx, dy;
    Sobel( image, dx, CV_32F, 1, 0, 3 );
    Sobel( image, dy, CV_32F, 0, 1, 3 );
    double normdx = norm( dx );
    double normdy = norm( dy );
    double sumSq = normdx * normdx + normdy * normdy;
    return static_cast<float>( 10000. / ( sumSq / image.size().area() ));
}

cvUtil::cvUtil(){
    b = ORB::create();
    prePts.reserve(4);
    curPts.reserve(4);
    iniPts.reserve(4);
    
    initialImgBuf.reserve(5);
    initialImgBuf.push_back(Mat());
    cout<< "buffer size is " << initialImgBuf.size() << endl;
    homoList.reserve(10);
    listpt = 0;
    
    preHomo0 = Mat();
    
    blurScale=5;
    countFromInitial=0;
    CountFromPrevious=0;
    countBlurSkip=0;
}

cvUtil::~cvUtil(){
    
}

bool cvUtil::setInitialPts(int left, int right, int top, int buttom, int w, int h){
    this->left=left;
    this->right=right;
    this->top=top;
    this->buttom=buttom;
    
    sepLeft=left;
    sepRight=right;
    sepTop=top;
    sepButtom=buttom;
    
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

//find the best iamge of first 5 as initial image with least blurness
bool cvUtil::initialImageFeature(Mat image){
    if(image.rows*image.cols<=0)
    {
        cout << "no image" << endl;
        return false;
    }
    
    cout<< "buffer size is " << initialImgBuf.size() << endl;
    if(initialImgBuf.size()<5){
        initialImgBuf.push_back(image);
        return false;
    }

    int initial=1;
    double minBlur=contrast_measure(initialImgBuf[1]);
    cout << "blurness 1 is " << minBlur << endl;
    for(int i=2; i<5; i++){
        cout << "blurness " << i << " is " << contrast_measure(initialImgBuf[i]) << endl;
        if(contrast_measure(initialImgBuf[i])<minBlur){
            minBlur=contrast_measure(initialImgBuf[i]);
            initial=i;
        }
    }
    
    this->img0=initialImgBuf[initial].clone();
    blurThreshold=minBlur*blurScale;
    countBlurSkip=0;
    
    clock_t startTime, endTime;
    startTime=clock();
    b->detectAndCompute(img0, Mat(), keyImg0, descImg0, false);
    endTime = clock();
    cout << "first image feature detect time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    
    cout << img0.type() << endl;
    cout << img0.at<Vec4b>(0, 0) << endl;
    
    return true;
}

double cvUtil::imageDiff(Mat img1, Mat img2){
    double diffsum=0;
    clock_t startTime, endTime;
    startTime=clock();
    //cout << img1.rows << ", " << img1.cols << endl;
    for(int i=0; i<img1.rows; i++){
        for(int j=0; j<img1.cols;j++){
            for(int k=0; k<4; k++){
//                cout << img1.at<Vec4b>(0, 0) << endl;
//                cout << img2.at<Vec4b>(0, 0) << endl;
                diffsum += abs(img1.at<Vec4b>(i, j)[k] - img2.at<Vec4b>(i, j)[k])/255.0;
            }
        }
    }
    endTime=clock();
    cout << "image diff detect time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    
    return diffsum;
}

bool cvUtil::addNewPoints(){
    return preOptPts.size() <= 30;
}

bool cvUtil::acceptTrackedPoint(int i){
    return status[i] && ((abs(preOptPts[i].x - curOptPts[i].x) + abs(preOptPts[i].y - curOptPts[i].y)) > 2);
}

int cvUtil::tracking(Mat curframe){
    if(curframe.rows*curframe.cols <=0 ){
        cout << "no frame" << endl;
        return 0;
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
    int moveNum=0;
    for (size_t i=0; i<curOptPts.size(); i++){
        //line(curframe, initial[i], curOptPts[i], Scalar(0, 0, 255));
        if(abs(initial[i].x-curOptPts[i].x)+abs(initial[i].y-curOptPts[i].y)>5){
            moveNum++;
        }
    }
    
    // 把当前跟踪结果作为下一此参考
    swap(curOptPts, preOptPts);
    swap(curGray, preGray);
    
    return moveNum;
}

cv::Mat cvUtil::homoMatrixToInitial(Mat img2){
    CvPoint upLeft=cvPoint(left, top);
    CvPoint lowRight=cvPoint(right, buttom);
    
    clock_t startTime, endTime;
    
    if(img0.rows*img0.cols<=0 || img2.rows*img2.cols<=0)
    {
        cout << "no img0 or img2" << endl;
        return preHomo0;
    }
    
    if(contrast_measure(img2)>blurThreshold){
        countBlurSkip++;
        return preHomo0;
    }
    
    //small movement
    int numm=tracking(img2);
    cout << "move num is " << numm << endl;
    if(preHomo0.cols==3 && numm<5){
        cout << "============" << endl;
        return preHomo0;
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
        
        if(bestMatches.size()>=featurePointNum) break;
    }
    
    Mat tempHomo=homo0;
    try{
        homo0=findHomography(imagePoints0,imagePoints2,CV_RANSAC);
    }
    catch(Exception& e){
        return preHomo0;
    }
    preHomo0=tempHomo;
    
    cout<<"Transform Matrix：\n"<<homo0<<endl<<endl;
    endTime = clock();
    cout << "homo matrix calculate time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    cout << "blur num is " << countBlurSkip << endl;

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
            return Mat();
        }
        else{
            img1=img0.clone();
            keyImg1=keyImg0;
            descImg1=descImg0.clone();
            flag=0;
        }
    }
    if(img2.rows*img2.cols<=0)
    {
        cout << "no img2" << endl;
        return preHomo;
    }

    if(contrast_measure(img2)>blurThreshold){
        countBlurSkip++;
        return preHomo;
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
        
        if(bestMatches.size()>=featurePointNum) break;
    }
    
    Mat homo=Mat();
    try{
        homo=findHomography(imagePoints1,imagePoints2,CV_RANSAC);
    }
    catch (Exception& e){
        return homo;
    }
    
    cout<<"Transform Matrix：\n"<<homo<<endl<<endl;
    endTime = clock();
    cout << "homo matrix calculate time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    
    if(flag)
    {
        try{
            homo=preHomo*homo;
        }catch(Exception &e){
            return preHomo;
        }
    }
    perspectiveTransform(iniPts, curPts, homo);
 
    img1=img2.clone();
    keyImg1=keyImg2;
    descImg1=descImg2.clone();
    prePts=curPts;
    preHomo=homo.clone();
    
    return homo;
}

Mat cvUtil::homoMatrixCombine(Mat img2, int flag){
    cout << "===============" << endl;
    cout << "feature num is " << tracking(img2) << endl;
    
    Mat homo1=homoMatrixToInitial(img2);
    Mat homo2=homoMatrixToPrevious(img2);
    
//    cout << "homo 1 : " << "\n" << homo1 << "\n" << endl;
//    cout << "homo 2 : " << "\n" << homo2 << "\n" << endl;
//    cout << "diff : " << "\n" << diff << "\n" << endl;
    
    double diff1and2=homoDiffSum(homo1, homo2);
    double diff1=homoDiffSum(homo1, preHomo0);
    double diff2=homoDiffSum(homo2, preHomo);
//    cout << "diff1and2 is " << diff1and2 << endl;
//    cout << "diff1 is " << diff1 << endl;
//    cout << "diff2 is " << diff2 << endl;

    //if(preHomo2.rows==3 && preHomo2.cols==3 && tracking(img2)<5) return preHomo2;
    
    Mat homo;
    
    if(flag==1){
//        if(homo1.at<double>(2,2)<0 && homo2.at<double>(2,2)<0){
//            return preHomo0;
//        }
//        else if(homo2.at<double>(2,2)<0 || diff1and2>1.1){
//            countFromInitial++;
//            preHomo=homo1.clone();
//            homo = homo1.clone();
//        }
//        else{
//            //homo = (homo1 + homo2)/2;
//            CountFromPrevious++;
//            homo=homo2.clone();
//        }
        if(homo1.cols<2 && homo2.cols<0){
            return preHomo0;
        }
        if(homo2.cols<2){
            countFromInitial++;
            preHomo=homo1.clone();
            homo = homo1.clone();
        }
        else if(homo1.cols<2 || diff2<diff1){
            //homo = (homo1 + homo2)/2;
            CountFromPrevious++;
            homo=homo2.clone();
        }
        else{
            countFromInitial++;
            preHomo=homo1.clone();
            homo = homo1.clone();
        }
    }
    else if(flag==2){
        if(homo1.cols<2 && homo2.cols<2){
            return preHomo0;
        }
        else if(homo2.cols<2 || diff1and2<1.5){
            countFromInitial++;
            preHomo=homo1.clone();
            homo = homo1.clone();
        }
        else{
            //homo = (homo1 + homo2)/2;
            CountFromPrevious++;
            homo=homo2.clone();
        }
    }
    
    cout << "from initial num is " << countFromInitial << ", from previous num is " << CountFromPrevious << endl;
    preHomo2=homo;
    return homo;
}

Mat cvUtil::filterHomo(Mat img2){
    Mat curHomo=homoMatrixCombine(img2, 1);
    Mat sumhomo=Mat(3,3,CV_64F);
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            sumhomo.at<double>(i,j)=0;
        }
    }
    cout << "initial sumhomo is " << "\n" << sumhomo << endl;
    
    for(int i=0; i<homoList.size();i++){
        sumhomo += homoList[i];
    }
    
    Mat avgHomo = sumhomo/homoList.size();
    if(homoDiffSum(avgHomo, curHomo)>1){
        return Mat();
    }
    
    if(homoList.size()<10){
        homoList.push_back(curHomo);
    }
    else{
        if(listpt>=10) listpt %= 10;
        homoList[listpt]=curHomo;
    }
    
    listpt++;
    
    return (0.2*sumhomo+0.8*curHomo);
}

double cvUtil::homoDiffSum(Mat homo1, Mat homo2){
    if(homo1.rows<2 || homo1.cols <2 || homo2.rows<2 || homo2.cols <2){
        return -1;
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

Mat cvUtil::grabCut(Mat image){
    Mat mask;
    mask.create(image.size(), CV_8UC1);
//    cv::Rect rect = cv::Rect( 190, 255, 145, 245);
    cv::Rect rect = cv::Rect(sepLeft, sepTop, sepRight-sepLeft, sepButtom-sepTop);
    cout << "(left, right, top, buttom) is (" << sepLeft << ", " << sepRight << ", " <<sepTop << ", " << sepButtom << endl;
    
    int rectW=right-left, rectH=buttom-top;
    
    clock_t startTime, endTime;
    startTime=clock();
    
    mask.setTo( GC_BGD );
    rect.x = max(0, rect.x);
    rect.y = max(0, rect.y);
    rect.width = min(rect.width, image.cols-rect.x);
    rect.height = min(rect.height, image.rows-rect.y);
    
    if(rect.width<=0) rect.width=1;
    if(rect.width<=rectW){
        rect.x -= (rectW-rect.width)/2;
        rect.width += (rectW-rect.width)/2;
    }
    
    if(rect.height<=0) rect.height=1;
    if(rect.height<=rectH){
        rect.y -= (rectH-rect.height)/2;
        rect.height += (rectH-rect.height)/2;
    }
    
    rect.x = max(0, rect.x);
    rect.y = max(0, rect.y);
    rect.width = min(rect.width, image.cols-rect.x);
    rect.height = min(rect.height, image.rows-rect.y);
    
    (mask(rect)).setTo(Scalar(GC_PR_FGD) );
    
    Mat bgdModel, fgdModel;
    //image must have CV_8UC3 type in function grabCut
    cout << "image type is " << image.type() << endl;
    cvtColor(image, image, COLOR_BGRA2BGR);
    cout << "image type is " << image.type() << endl;
    cv::grabCut(image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_RECT);
    endTime=clock();
    cout << "grabcut time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    
    //cout << "post mask is " << mask << endl;
    Mat binMask;
    getBinMask(mask, binMask );
    
    Mat dstImg;
    image.copyTo(dstImg, binMask);
    
    //rectangle(dstImg, cvPoint(sepLeft, sepTop), cvPoint(sepRight, sepButtom), cvScalar(1), 3);
    
    //create new border by binMask
    sepTop=height-1;
    sepButtom=0;
    sepLeft=width-1;
    sepRight=0;
    cout << "(left, right, top, buttom) is (" << sepLeft << ", " << sepRight << ", " <<sepTop << ", " << sepButtom << endl;
    
    for(int j=0; j<width; j++)
    {
        for(int i=0; i<height; i++)
        {
            if(binMask.at<uchar>(i,j)){
                if(sepTop>i) sepTop=i;
                break;
            }
        }
        for(int i=height-1; i>=0; i--)
        {
            if(binMask.at<uchar>(i,j)){
                if(sepButtom<i) sepButtom=i;
                break;
            }
        }
    }
    
    for(int i=0; i<height; i++)
    {
        for(int j=0; j<width; j++)
        {
            if(binMask.at<uchar>(i,j)){
                if(sepLeft>j) sepLeft=j;
                break;
            }
        }
        for(int j=width-1; j>=0; j--)
        {
            if(binMask.at<uchar>(i,j)){
                if(sepRight<j) sepRight=j;
                break;
            }
        }
    }
    cout << "(left, right, top, buttom) is" << sepLeft << ", " << sepRight << ", " <<sepTop << ", " << sepButtom << endl;
    
    return dstImg;
}

bool cvUtil::reset(){
    
    return true;
}


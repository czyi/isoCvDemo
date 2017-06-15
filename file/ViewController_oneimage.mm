//
//  ViewController.m
//  cvdemo
//
//  Created by Ziyi Chen on 5/5/17.
//  Copyright © 2017 Ziyi Chen. All rights reserved.
//

#import "ViewController.h"
#import "SecondViewController.h"
#import <Masonry.h>

#import <UIKit/UIKit.h>
#import <opencv2/opencv.hpp>
#import <opencv2/imgproc/types_c.h>
#import <opencv2/imgcodecs/ios.h>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#import <string>
#import <vector>

using namespace std;
using namespace cv;

@interface ViewController ()
{

}

@end

@implementation ViewController

    UILabel* runtimeLab;

Mat tranformMatrix(vector<String>& fileName, CvPoint upLeft, CvPoint lowRight)
{
    clock_t startTime, endTime;
    
    Mat img1=imread(fileName[0], IMREAD_GRAYSCALE);
    Mat img2=imread(fileName[1], IMREAD_GRAYSCALE);
    
    if(img1.rows*img1.cols<=0)
    {
        cout << "no img1" << endl;
        //return NULL;
    }
    
    if(img2.rows*img2.cols<=0)
    {
        cout << "no img1" << endl;
        //return NULL;
    }
    
    Ptr<Feature2D> b;
    
    Ptr<DescriptorMatcher> descriptorMatcher;
    vector<DMatch> matches;
    vector<KeyPoint> keyImg1, keyImg2;
    Mat descImg1, descImg2;
    
    b = ORB::create(500);
    
    startTime=clock();
    b->detectAndCompute(img1, Mat(), keyImg1, descImg1, false);
    endTime = clock();
    cout << "first image feature detect time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    [runtimeLab setText:[runtimeLab.text stringByAppendingString:[NSString stringWithFormat:@"\nfirst image feature detect time :  %f s",(double)(endTime - startTime) / CLOCKS_PER_SEC]]];
    
    startTime=clock();
    b->detectAndCompute(img2, Mat(),keyImg2, descImg2, false);
    endTime = clock();
    cout << "second image feature detect time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    [runtimeLab setText:[runtimeLab.text stringByAppendingString:[NSString stringWithFormat:@"\nsecond image feature detect time :  %f s",(double)(endTime - startTime) / CLOCKS_PER_SEC]]];
    
    startTime=clock();
    descriptorMatcher = DescriptorMatcher::create("BruteForce-Hamming");
    descriptorMatcher->match(descImg1, descImg2, matches, Mat());
    endTime = clock();
    cout << "feature matching time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    [runtimeLab setText:[runtimeLab.text stringByAppendingString:[NSString stringWithFormat:@"\nfeature matching time :  %f s",(double)(endTime - startTime) / CLOCKS_PER_SEC]]];
    
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
        if(kp.pt.x < upLeft.x || kp.pt.x > lowRight.x || kp.pt.y < upLeft.y || kp.pt.y >lowRight.y) continue;
        
        DMatch current=matches[index.at<int>(i, 0)];
        bestMatches.push_back(current);
        
        imagePoints1.push_back(keyImg1[current.queryIdx].pt);
        imagePoints2.push_back(keyImg2[current.trainIdx].pt);
        
        if(bestMatches.size()>=30) break;
        //if(imagePoints1.size()>=30) break;
    }
    
    Mat homo=findHomography(imagePoints1,imagePoints2,CV_RANSAC);
    cout<<"Transform Matrix：\n"<<homo<<endl<<endl;
    endTime = clock();
    cout << "homo matrix calculate time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    [runtimeLab setText:[runtimeLab.text stringByAppendingString:[NSString stringWithFormat:@"\nhomo matrix calculate time :  %f s",(double)(endTime - startTime) / CLOCKS_PER_SEC]]];
    
    Mat result;
    rectangle(img1, upLeft, lowRight, cvScalar(1), 3);
    drawMatches(img1, keyImg1, img2, keyImg2, bestMatches, result);
    //    imshow("result", result);
    //    imwrite("result.jpg",result);
    
    vector<Point2d> rectPts(4), transPts(4);
    rectPts[0]=upLeft;
    rectPts[1]=CvPoint(upLeft.x, lowRight.y);
    rectPts[2]=lowRight;
    rectPts[3]=CvPoint(lowRight.x, upLeft.y);
    
    cv::Point points[1][4];
    perspectiveTransform(rectPts, transPts, homo);
    for(int i=0; i<4; i++)
    {
        //cout << transPts[i] << endl;
        points[0][i]=transPts[i];
    }
    
    const cv::Point* pt[1] = { points[0] };
    int npt[1] = {4};
    //polylines(img2, pt, npt, 1, 1, Scalar(250,0,0)) ;
    polylines(img2, pt, npt, 1, 1, Scalar(1),3);
    //imshow("img2_poly", img2);
    
    Mat transResult(img1.rows,img1.cols + img2.cols+1,img1.type());
    img1.colRange(0,img1.cols).copyTo(transResult.colRange(0,img1.cols));
    img2.colRange(0,img2.cols).copyTo(transResult.colRange(img2.cols+1,transResult.cols));
    //    imshow("tansResult",transResult);
    //    imwrite("tansResult.jpg",transResult);
    
    //waitKey();
    return transResult;
}


- (void)viewDidLoad {
    
    [super viewDidLoad];
    
    //NSLog(@"test");
    //self.view.backgroundColor = [UIColor blackColor];
    
    UIImageView* imgView = [UIImageView new];
    runtimeLab = [UILabel new];
    [runtimeLab setNumberOfLines:0];
    [runtimeLab setFont:[UIFont systemFontOfSize:11]];
    [runtimeLab setBackgroundColor:[UIColor whiteColor]];
    
    [imgView setContentMode:UIViewContentModeScaleAspectFit];
    [self.view addSubview:imgView];
    
    [imgView mas_remakeConstraints:^(MASConstraintMaker *make) {
        make.center.width.equalTo(self.view);
        
    }];
    
    [self.view addSubview:runtimeLab];
    [runtimeLab mas_remakeConstraints:^(MASConstraintMaker *make) {
       make.centerX.equalTo(self.view);
        make.bottom.equalTo(self.view).offset(-10);
//        make.height.equalTo(@30);
    }];
    [runtimeLab setTextColor:[UIColor blackColor]];
    [runtimeLab setText:@""];
    
    
//    UIImage *image = [UIImage imageNamed:@"lena.jpg"];
//    imgView.image = image;
    
    vector<String> fileName;
    fileName.push_back([[[NSBundle mainBundle] pathForResource:@"initialSmall01" ofType:@"jpg"] cStringUsingEncoding:NSUTF8StringEncoding]);
    fileName.push_back([[[NSBundle mainBundle] pathForResource:@"shoes01Small01" ofType:@"jpg"] cStringUsingEncoding:NSUTF8StringEncoding]);
    
    clock_t startTime, endTime;
    startTime = clock();
    Mat resImg = tranformMatrix(fileName, CvPoint(430, 600), CvPoint(620, 900));
    endTime = clock();
    [runtimeLab setText:[runtimeLab.text stringByAppendingString:[NSString stringWithFormat:@"\ntime : %f s",(double)(endTime - startTime) / CLOCKS_PER_SEC]]];
    
    imgView.image = MatToUIImage(resImg);
    
//    [self.view setNeedsLayout];
    
    NSLog(@"end");
}

-(void)viewDidAppear:(BOOL)animated{
    [super viewDidAppear:animated];
//    NSLog(@"viewDidAppear");
//    self.view.backgroundColor=[UIColor redColor];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
    
}


@end

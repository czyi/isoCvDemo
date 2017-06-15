//
//  ViewController.m
//  orbslamios
//
//  Created by Ying Gaoxuan on 16/11/1.
//  Copyright © 2016年 Ying Gaoxuan. All rights reserved.
//

#import "ViewController.h"
#include "Masonry.h"



#import "cvUtiil.hpp"




@interface ViewController ()<CvVideoCameraDelegate>
{
    CvVideoCamera* videoCamera;
    UIImageView* imgView;
    UIButton* startBtn;
    UIButton* resetBtn;
    cvUtil* calMatrix;
    int state;
    int method;
}

@end

@implementation ViewController

//@synthesize videoCamera = _videoCamera;



- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    [self.view setBackgroundColor:[UIColor whiteColor]];
    imgView = [[UIImageView alloc] initWithImage:[UIImage new]];
    [self.view addSubview:imgView];
    
    [imgView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.right.equalTo(self.view);
        make.top.equalTo(self.view).with.offset(80);
        make.bottom.equalTo(self.view).with.offset(-100);
    }];
    
    startBtn = [[UIButton alloc] init];
    [startBtn addTarget:self action:@selector(startButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    [startBtn setTitle:@"Start" forState:UIControlStateNormal];
    [startBtn setBackgroundColor:[UIColor redColor]];
    [self.view addSubview:startBtn];
    [startBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.right.equalTo(imgView);
        make.top.equalTo(imgView.mas_bottom);
        make.bottom.equalTo(self.view).with.offset(-50);
        
    }];
    
    resetBtn = [[UIButton alloc] init];
    [resetBtn addTarget:self action:@selector(resetButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    [resetBtn setTitle:@"Reset" forState:UIControlStateNormal];
    [resetBtn setBackgroundColor:[UIColor blackColor]];
    [self.view addSubview:resetBtn];
    [resetBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.right.equalTo(imgView);
        make.top.equalTo(imgView.mas_bottom).with.offset(50);
        make.bottom.equalTo(self.view);
        
    }];
    
    videoCamera = [[CvVideoCamera alloc] initWithParentView:imgView];
    videoCamera.delegate = self;
    videoCamera.defaultAVCaptureDevicePosition = AVCaptureDevicePositionBack;
    videoCamera.defaultAVCaptureSessionPreset = AVCaptureSessionPreset640x480;
    videoCamera.defaultFPS = 30;
    videoCamera.grayscaleMode = NO;
    
    state=0;
    method=1;
    calMatrix=new cvUtil();
}

-(void)viewDidAppear:(BOOL)animated{
    [super viewDidAppear:animated];
    [videoCamera start];
    NSLog(@"DemoType = %d",self.demoType);
}


- (void)startButtonPressed:(id)sender
{
    //[videoCamera start];
    state=1;
}

- (void)resetButtonPressed:(id)sender
{
    //[videoCamera start];
    state=0;
}

- (void)processImage:(cv::Mat &)image
{    
    int left = self.view.frame.size.height/4;
    int right = 3*self.view.frame.size.height/4;
    int top  = 1.5*self.view.frame.size.width/4;
    int buttom = 3.5*self.view.frame.size.width/4;
    
    if(state==0)
    {
        rectangle(image, cvPoint(left, top), cvPoint(right, buttom), cvScalar(1), 3);
    }
    else if(state==1)
    {
        calMatrix->initialImageFeature(image);
        calMatrix->setInitialPts(left, right, top, buttom);
        state=2;
    }
    else if(state==2)
    {
        //if(method==0)
        if(self.demoType==CVDemoTypeDefault)
        {//compare to first image
            Mat homo=calMatrix->homoMatrixToInitial(image, cvPoint(left, top), cvPoint(right, buttom));
            
            vector<Point2d> rectPts(4), transPts(4);
            rectPts[0]=cvPoint(left+50, top+30);
            rectPts[1]=cvPoint(left+50, buttom-30);
            rectPts[2]=cvPoint(right-50, buttom-30);
            rectPts[3]=cvPoint(right-50, top+30);
            
            cv::Point points[1][4];
            perspectiveTransform(rectPts, transPts, homo);
            for(int i=0; i<4; i++)
            {
                points[0][i]=transPts[i];
            }
            
            const cv::Point* pt[1] = { points[0] };
            int npt[1] = {4};
            polylines(image, pt, npt, 1, 1, Scalar(1),3);
        }
        //else if(method==1)
        else if(self.demoType==CVDemoTypeIncremental)
        {// compare to prevous image
            image=calMatrix->homoMatrixToPrevious(image);
        }
    }
    
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end

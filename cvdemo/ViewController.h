//
//  ViewController.h
//  cvdemo
//
//  Created by Ziyi Chen on 5/5/17.
//  Copyright © 2017 Ziyi Chen. All rights reserved.
//

#import <UIKit/UIKit.h>


enum CVDemoType {
    CVDemoTypeDefault,
    CVDemoTypeIncremental,
};

@interface ViewController : UIViewController

@property(assign) enum CVDemoType demoType;

//@property (nonatomic, strong) CvVideoCamera* videoCamera;

@end


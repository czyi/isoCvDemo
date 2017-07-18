# isoCvDemo
- Feature point tracking

## Demo
- https://youtu.be/hWySuIgFSls

## Environment
- Use Pod to install Masonry and OpenCV (default version).

## How to use
- Currently it only supports slow movement of camera or shoes.
- Also error will be accumulated.

### Main Interface
![](https://github.com/czyi/isoCvDemo/blob/master/file/MainInterface.PNG)
- MainMenuViewController.m
- The last `Optical Flow Demo` is current version.

### Tracking Interface
![](https://github.com/czyi/isoCvDemo/blob/master/file/TrackingInterface.PNG)
- ViewController.mm
- The brightness of image is fixed.
- Clicking `reset` buttom can adjust image to current light.

### Initialization
- Put shoes into the retangle and click `start` button to start tracking.
- Click `reset` buttom to back restart initialization.

### Camera Pose Matrix (4x4)
- cvOpticalFlow.cpp
- function trackingFrontFromInitialNineGrid
- `avgHomo` is the homography matrix of shoes plane in two frame (initial frame and current frame)
- `R` and `t` is the rotation matrix and transformation vector of camera pose
- Pose Matrix is saved in `pose` and printed

## Note
The code is not up to date.










#include "DriveCamera.h"

bool openCamera(void)
{
    VideoCapture cap;
  
    cap.open(0);
    if (!cap.isOpened()) {
        return false;
    }
  
    Mat frame;
  
    while (true) {
	cap >> frame;
	if (frame.empty()) {
	    break;
	}
    
	imshow("video", frame);
	if (waitKey(20) > 0) {
	    break;
	}
    }
    cap.release();
    destroyAllWindows();
  
    return true;
}
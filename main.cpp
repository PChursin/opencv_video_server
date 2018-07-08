#include "opencv2/opencv.hpp"
#include <iostream>

using namespace std;
using namespace cv;

int main(){

    VideoCapture cap(0);

    if(!cap.isOpened()){
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    while(true){
        Mat frame;
        cap >> frame;
        if (frame.empty())
            break;
        imshow( "Frame", frame );

        // Press  ESC on keyboard to exit
        auto c = (char)waitKey(25);
        if(c == 27)
            break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
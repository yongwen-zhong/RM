#include <iostream>
#include<opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
    Mat image;
    VideoCapture capture(0);
    capture.read(image);

    imshow("1",image);


    Mat hsv;
    cvtColor(image,hsv,COLOR_BGR2HSV);
    imshow("2",hsv);

    Mat mask;
    inRange(hsv,Scalar(100,43,46),Scalar(124,255,255),mask);
//    imshow("3",mask);

    bitwise_not(mask,mask);
//    imshow("4",mask);

    Mat redBack = Mat::zeros(image.size(),image.type());
    redBack = Scalar(60,60,210);
//    imshow("5",redBack);

    image.copyTo(redBack,mask);
//    imshow("6",redBack);

    waitKey(0);
    return 0;
}

#include <iostream>
#include<opencv2/opencv.hpp> 
using namespace cv;//命名空间
using namespace std;

int main(int argc, char *argv[])
{

    Mat bkimage;                                          
    Mat frame;                                            
    Mat blurImg;                                          
    VideoCapture capture(0);                              
    capture.read(bkimage);                                
    imshow("1",bkimage);

    while(capture.read(frame))
    {
        imshow("video",frame);
        blur(frame,blurImg,Size(50,50));//模糊处理
//        imshow("blur",blurImg);
        //2.图片转Hsv格式
        Mat hsv;
        cvtColor(frame,hsv,COLOR_BGR2HSV);
//        imshow("hsv",hsv);

        //3.识别出红色区域   红色有2个区域
        Mat mask,mask1;
        inRange(hsv,Scalar(0,120,100),Scalar(10,255,255),mask);
        inRange(hsv,Scalar(170,120,100),Scalar(180,255,255),mask1);
    //    imshow("mask",mask);
    //    imshow("mask1",mask1);

        mask = mask + mask1 ; //红布区域内模板
//        imshow("mask",mask);

        //4.取反操作
        Mat uMask;//红布区域以外的模板
        bitwise_not(mask,uMask);
//        imshow("uMask",uMask);

       
        Mat bkMask;
        bitwise_and(bkimage,bkimage,bkMask,mask);
//        imshow("bkMask",bkMask);

        Mat bkUmask;
        bitwise_and(frame,frame,bkUmask,uMask);
//        imshow("bkUmask",bkUmask);

        //像素相融合
        Mat final;
    //    add(bkMask,bkUmask,final);//
                                        
                                    
        final = bkMask + bkUmask;
        imshow("final",final);


        waitKey(30);
    }


    return 0;
}


    


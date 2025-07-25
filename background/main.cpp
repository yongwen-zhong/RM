#include <iostream>
#include<opencv2/opencv.hpp> //引入头文件

using namespace cv;//命名空间
using namespace std;

//opencv机器视觉库，C/C++编写出来的，提供很多功能给我们使用，都是 函数的形式提供给我们

int main(int argc, char *argv[])
{

    Mat bkimage;                                          //背景图片
    Mat frame;                                            //保存每一帧摄像头图片
    Mat blurImg;                                          //模糊图片
    VideoCapture capture(0);                              //当参数为0的时候，表示调用电脑本地默认摄像头
    capture.read(bkimage);                                //将初始照片赋给bkimage
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

        mask = mask + mask1 ; //此时mask就是完整的红布区域内模板
//        imshow("mask",mask);

        //4.取反操作
        Mat uMask;//红布区域以外的模板
        bitwise_not(mask,uMask);
//        imshow("uMask",uMask);

        //5.   背景图片 + mask扣1   uMask扣2
        Mat bkMask;
        bitwise_and(bkimage,bkimage,bkMask,mask);
//        imshow("bkMask",bkMask);

        Mat bkUmask;
        bitwise_and(frame,frame,bkUmask,uMask);
//        imshow("bkUmask",bkUmask);

        //6.最终2个图片 像素相融合
        Mat final;
    //    add(bkMask,bkUmask,final);// +可以吗？  可以1  不可以2  add函数有什么存在的意义？
                                        // 一个程序员 开发一个功能 实现路径很多种
                                    //新手 - 熟的函数、方式    老鸟-项目迭代性、运行效率
        final = bkMask + bkUmask;
        imshow("final",final);


        waitKey(30);//参数0-等待用户按下任意键   30 >0  表示休眠30毫秒  1000毫秒=1秒
    }


    return 0;//程序正常结束
}

/*

    //1.显示2张图片
    Mat backImg = imread("d:/background.jpg");//背景图片
    Mat hideImg = imread("d:/hide.jpg");//人物拿着红布图片

    imshow("back",backImg);
    imshow("hide",hideImg);

    //2.图片转Hsv格式
    Mat hsv;
    cvtColor(hideImg,hsv,COLOR_BGR2HSV);
    imshow("hsv",hsv);

    //3.识别出红色区域   红色有2个区域
    Mat mask,mask1;
    inRange(hsv,Scalar(0,120,100),Scalar(10,255,255),mask);
    inRange(hsv,Scalar(170,120,100),Scalar(180,255,255),mask1);
//    imshow("mask",mask);
//    imshow("mask1",mask1);

    mask = mask + mask1 ; //此时mask就是完整的红布区域内模板
    imshow("mask",mask);

    //4.取反操作
    Mat uMask;//红布区域以外的模板
    bitwise_not(mask,uMask);
    imshow("uMask",uMask);

    //5.   背景图片 + mask扣1   uMask扣2
    Mat bkMask;
    bitwise_and(backImg,backImg,bkMask,mask);
    imshow("bkMask",bkMask);

    Mat bkUmask;
    bitwise_and(hideImg,hideImg,bkUmask,uMask);
    imshow("bkUmask",bkUmask);

    //6.最终2个图片 像素相融合
    Mat final;
//    add(bkMask,bkUmask,final);// +可以吗？  可以1  不可以2  add函数有什么存在的意义？
                                    // 一个程序员 开发一个功能 实现路径很多种
                                //新手 - 熟的函数、方式    老鸟-项目迭代性、运行效率
    final = bkMask + bkUmask;
    imshow("final",final);



    //1.显示图片
    Mat image = imread("d:/test.png");
    imshow("1",image);

    //2.转换成hsv格式
    Mat hsv;
    cvtColor(image,hsv,COLOR_BGR2HSV);
    imshow("2",hsv);

    //3.截取颜色空间范围 hsv范围内 呈白色模板
    Mat mask;
    inRange(hsv,Scalar(100,43,46),Scalar(124,255,255),mask);
    imshow("3",mask);

    //4.现在可以抠出背景，但其实我们想抠人像
    //取反操作
    bitwise_not(mask,mask);
    imshow("4",mask);//此时此刻的mask人像抠图模板

    //5.生成一张红色背景图   大小尺寸？  类型？
                       // :: C++ 作用域分解运算符  zeros() 创建制定几行几列的矩阵  像素为0
    Mat redBack = Mat::zeros(image.size(),image.type());
    //210 67 115
    redBack = Scalar(40,40,200); //BGR
    imshow("5",redBack);

    //6.图片的拷贝操作
    image.copyTo(redBack,mask);
    imshow("6",redBack);




    //1.显示1张图片
    Mat img = imread("d:/niu0.jpg"); //Mat类型 图片类型  容器对象类型
    imshow("title",img);//显示一张图片

    //2.图片做模糊处理
    Mat blurImg;//模糊图片对象
    blur(img,blurImg,Size(50,50));//模糊处理函数  size(横向模糊尺寸，纵向模糊尺寸)
    imshow("blur",blurImg);

    //3.图片做灰度处理
    Mat grayImg;//灰色图片对象
    cvtColor(img,grayImg,COLOR_BGR2GRAY);//颜色处理函数
    imshow("gray",grayImg);

    //4.图片做二值化处理
    Mat threImg;//二值化图片
    threshold(grayImg,threImg,160,255,THRESH_BINARY);//二值化处理函数
    imshow("thre",threImg);

*/

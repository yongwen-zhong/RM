#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;
const int min_w = 80;  // 最小宽度
const int max_w = 300; // 最大宽度
const int min_h = 80;  // 最小高度
const int max_h = 300; // 最大高度

int main() {
    namedWindow("video", WINDOW_KEEPRATIO);
    VideoCapture cap("D:/cppworkapsce/cv_carrs/cars.mp4");

    // 获取视频的帧率,计算延迟
    double fps = cap.get(CAP_PROP_FPS);
    int delay = static_cast<int>(1000.0 / fps);

    // 创建高斯混合模型背景减除器
    Ptr<BackgroundSubtractorMOG2> bgsubmog = createBackgroundSubtractorMOG2(2000, 30.0, false);

    //创建形态学模型
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));

    // 计数相关变量
    int carno = 0;
    const int line_high = 500; 
    const int offset = 30;  
    vector<Point> cars;  

    while (true) {
        Mat frame;
        cap.read(frame);

        // 灰度转换
        Mat grayFrame;
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

        // 高斯去噪
        Mat blurredFrame;
        GaussianBlur(grayFrame, blurredFrame, Size(5,5), 10);

        // 去背景
        Mat mask;
        bgsubmog->apply(blurredFrame, mask);

        // 形态学操作
        erode(mask, mask, kernel);

        morphologyEx(mask, mask, MORPH_OPEN, kernel);
        morphologyEx(mask, mask, MORPH_CLOSE, kernel);

        // 中值滤波
        medianBlur(mask, mask, 3);

        // 二值化处理
        threshold(mask, mask, 127, 255, THRESH_BINARY);

        // 查找轮廓
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

        // 遍历轮廓并绘制矩形框
        for (size_t i = 0; i < contours.size(); i++) {
            Rect rect = boundingRect(contours[i]);
            int x = rect.x;
            int y = rect.y;
            int w = rect.width;
            int h = rect.height;

            // 对车辆的宽高进行判断
            bool isValid = (w >= min_w) && (w <= max_w) && (h >= min_h) && (h <= max_h);
            if (!isValid) {
                continue;
            }

            rectangle(frame, rect, Scalar(0, 0, 255), 2);

            // 计算并存储中心点
            int centerX = x + w / 2;
            int centerY = y + h / 2;
            Point cpoint(centerX, centerY);
            cars.push_back(cpoint);
        }


        // 遍历中心点数组，进行计数
        for (vector<Point>::iterator it = cars.begin(); it != cars.end(); ) {
            // 判断中心点是否通过参考线附近区域
            if (abs(it->y - line_high) <= offset) {
                carno++;
                it = cars.erase(it);  // 移除已计数的车辆
                cout << "Car count: " << carno << endl;
            }
            else {
                ++it;
            }
        }

        // 在画面上绘制参考线
        line(frame, Point(0, line_high), Point(frame.cols, line_high), Scalar(255, 0, 0), 2);

        // 在画面上显示计数结果
        string countText = "Count: " + to_string(carno);
        putText(frame, countText, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);

        // 显示绘制矩形框后的原始视频帧
        imshow("video", frame);

        int key = waitKey(delay);
        if (key == 27) {  // ESC键退出
            break;
        }
    }
    cap.release();
    destroyAllWindows();
    return 0;
}
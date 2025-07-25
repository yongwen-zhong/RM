#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace cv;
using namespace std;

// 计算两点间距离
double distance(Point a, Point b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

int main() {
    // 打开摄像头
    VideoCapture cap(0, CAP_DSHOW);
    if (!cap.isOpened()) {
        cerr << "无法打开摄像头！" << endl;
        return -1;
    }

    while (true) {
        Mat frame;
        cap.read(frame);
        if (frame.empty()) {
            cerr << "无法获取帧！" << endl;
            break;
        }

        flip(frame, frame, 1);  // 水平翻转（镜像显示）

        // 定义ROI区域（右上角200x200）
        Rect roi_rect(400, 10, 200, 200);
        Mat roi = frame(roi_rect);
        rectangle(frame, roi_rect, Scalar(0, 0, 255), 1);  // 标记ROI

        // ======================取消肤色检测，改用灰度阈值分割======================
        // 1. 转为灰度图（替代HSV肤色检测）
        Mat gray, mask;
        cvtColor(roi, gray, COLOR_BGR2GRAY);  // BGR转灰度

        // 2. 阈值分割（区分手和背景：假设手是较暗/较亮的区域，可根据实际调整）
        // 注意：这里用自适应阈值，比固定阈值更适应光照变化
        adaptiveThreshold(gray, mask, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 11, 2);
        // 说明：ADAPTIVE_THRESH_GAUSSIAN_C=高斯加权阈值；THRESH_BINARY_INV=黑白反转（手为白色，背景黑色）

        // ==========================================================================

        // 3. 预处理（膨胀+高斯模糊，保留原逻辑但降低强度，避免过度模糊）
        Mat kernel = getStructuringElement(MORPH_RECT, Size(2, 2));
        dilate(mask, mask, kernel, Point(-1, -1), 1);  // 降低膨胀次数（1次）
        GaussianBlur(mask, mask, Size(5, 5), 1);  // 降低模糊程度（sigma=1）

        // 4. 提取轮廓（原逻辑不变）
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

        string result = "未知";

        if (!contours.empty()) {
            // 筛选面积最大的轮廓（手部），并过滤小面积干扰
            double min_area = 500;  // 最小轮廓面积（过滤背景噪点）
            vector<vector<Point>> valid_contours;
            for (auto& c : contours) {
                if (contourArea(c) > min_area) {
                    valid_contours.push_back(c);
                }
            }
            if (valid_contours.empty()) {
                result = "无有效轮廓";
            }
            else {
                // 找到最大有效轮廓
                int max_idx = 0;
                double max_area = contourArea(valid_contours[0]);
                for (int i = 1; i < valid_contours.size(); i++) {
                    double area = contourArea(valid_contours[i]);
                    if (area > max_area) {
                        max_area = area;
                        max_idx = i;
                    }
                }
                vector<Point> cnt = valid_contours[max_idx];
                double areacnt = max_area;

                // 计算凸包和面积比
                vector<Point> hull;
                convexHull(cnt, hull);
                double areahull = contourArea(hull);
                double arearatio = areacnt / areahull;

                // 计算凸缺陷
                vector<int> hull_indices;
                convexHull(cnt, hull_indices, false);
                vector<Vec4i> defects;
                if (hull_indices.size() > 3) {
                    convexityDefects(cnt, hull_indices, defects);
                }

                // 统计有效凸缺陷（放宽条件，避免漏检）
                int n = 0;
                for (int i = 0; i < defects.size(); i++) {
                    Vec4i d = defects[i];
                    if (d[3] <= 0) continue;

                    int s = d[0], e = d[1], f = d[2];
                    Point start = cnt[s];
                    Point end = cnt[e];
                    Point far = cnt[f];
                    double depth = d[3] / 256.0;  // 缺陷深度

                    // 计算角度
                    double a = distance(end, start);
                    double b = distance(far, start);
                    double c = distance(end, far);
                    if (b == 0 || c == 0) continue;
                    double angle = acos((b * b + c * c - a * a) / (2 * b * c)) * 180 / CV_PI;

                    // 放宽条件：角度≤100度，深度>10（更容易检测到指缝）
                    if (angle <= 100 && depth > 10) {
                        n++;
                        circle(roi, far, 3, Scalar(255, 0, 0), -1);  // 标记缺陷点
                    }
                    line(roi, start, end, Scalar(0, 255, 0), 2);  // 绘制凸包边缘
                }

                // 5. 手势判断（原逻辑不变）
                if (n == 0) {
                    if (arearatio > 0.9) {
                        result = "0";  // 拳头
                    }
                    else {
                        result = "1";  // 单指
                    }
                }
                else if (n == 1) {
                    result = "2";
                }
                else if (n == 2) {
                    result = "3";
                }
                else if (n == 3) {
                    result = "4";
                }
                else if (n == 4) {
                    result = "5";
                }
                else {
                    result = "未知";
                }
            }
        }

        // 显示结果
        putText(frame, result, Point(400, 80), FONT_HERSHEY_SIMPLEX, 2.0, Scalar(0, 0, 255), 3);
        imshow("手势识别（0-5）", frame);
        imshow("阈值分割结果（调试用）", mask);  // 显示mask，便于观察分割效果

        // 按ESC退出
        if (waitKey(25) == 27) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
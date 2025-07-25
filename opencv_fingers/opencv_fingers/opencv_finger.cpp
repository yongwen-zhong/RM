#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace cv;
using namespace std;

// ������������
double distance(Point a, Point b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

int main() {
    // ������ͷ
    VideoCapture cap(0, CAP_DSHOW);
    if (!cap.isOpened()) {
        cerr << "�޷�������ͷ��" << endl;
        return -1;
    }

    while (true) {
        Mat frame;
        cap.read(frame);
        if (frame.empty()) {
            cerr << "�޷���ȡ֡��" << endl;
            break;
        }

        flip(frame, frame, 1);  // ˮƽ��ת��������ʾ��

        // ����ROI�������Ͻ�200x200��
        Rect roi_rect(400, 10, 200, 200);
        Mat roi = frame(roi_rect);
        rectangle(frame, roi_rect, Scalar(0, 0, 255), 1);  // ���ROI

        // ======================ȡ����ɫ��⣬���ûҶ���ֵ�ָ�======================
        // 1. תΪ�Ҷ�ͼ�����HSV��ɫ��⣩
        Mat gray, mask;
        cvtColor(roi, gray, COLOR_BGR2GRAY);  // BGRת�Ҷ�

        // 2. ��ֵ�ָ�����ֺͱ������������ǽϰ�/���������򣬿ɸ���ʵ�ʵ�����
        // ע�⣺����������Ӧ��ֵ���ȹ̶���ֵ����Ӧ���ձ仯
        adaptiveThreshold(gray, mask, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 11, 2);
        // ˵����ADAPTIVE_THRESH_GAUSSIAN_C=��˹��Ȩ��ֵ��THRESH_BINARY_INV=�ڰ׷�ת����Ϊ��ɫ��������ɫ��

        // ==========================================================================

        // 3. Ԥ��������+��˹ģ��������ԭ�߼�������ǿ�ȣ��������ģ����
        Mat kernel = getStructuringElement(MORPH_RECT, Size(2, 2));
        dilate(mask, mask, kernel, Point(-1, -1), 1);  // �������ʹ�����1�Σ�
        GaussianBlur(mask, mask, Size(5, 5), 1);  // ����ģ���̶ȣ�sigma=1��

        // 4. ��ȡ������ԭ�߼����䣩
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

        string result = "δ֪";

        if (!contours.empty()) {
            // ɸѡ��������������ֲ�����������С�������
            double min_area = 500;  // ��С������������˱�����㣩
            vector<vector<Point>> valid_contours;
            for (auto& c : contours) {
                if (contourArea(c) > min_area) {
                    valid_contours.push_back(c);
                }
            }
            if (valid_contours.empty()) {
                result = "����Ч����";
            }
            else {
                // �ҵ������Ч����
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

                // ����͹���������
                vector<Point> hull;
                convexHull(cnt, hull);
                double areahull = contourArea(hull);
                double arearatio = areacnt / areahull;

                // ����͹ȱ��
                vector<int> hull_indices;
                convexHull(cnt, hull_indices, false);
                vector<Vec4i> defects;
                if (hull_indices.size() > 3) {
                    convexityDefects(cnt, hull_indices, defects);
                }

                // ͳ����Ч͹ȱ�ݣ��ſ�����������©�죩
                int n = 0;
                for (int i = 0; i < defects.size(); i++) {
                    Vec4i d = defects[i];
                    if (d[3] <= 0) continue;

                    int s = d[0], e = d[1], f = d[2];
                    Point start = cnt[s];
                    Point end = cnt[e];
                    Point far = cnt[f];
                    double depth = d[3] / 256.0;  // ȱ�����

                    // ����Ƕ�
                    double a = distance(end, start);
                    double b = distance(far, start);
                    double c = distance(end, far);
                    if (b == 0 || c == 0) continue;
                    double angle = acos((b * b + c * c - a * a) / (2 * b * c)) * 180 / CV_PI;

                    // �ſ��������Ƕȡ�100�ȣ����>10�������׼�⵽ָ�죩
                    if (angle <= 100 && depth > 10) {
                        n++;
                        circle(roi, far, 3, Scalar(255, 0, 0), -1);  // ���ȱ�ݵ�
                    }
                    line(roi, start, end, Scalar(0, 255, 0), 2);  // ����͹����Ե
                }

                // 5. �����жϣ�ԭ�߼����䣩
                if (n == 0) {
                    if (arearatio > 0.9) {
                        result = "0";  // ȭͷ
                    }
                    else {
                        result = "1";  // ��ָ
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
                    result = "δ֪";
                }
            }
        }

        // ��ʾ���
        putText(frame, result, Point(400, 80), FONT_HERSHEY_SIMPLEX, 2.0, Scalar(0, 0, 255), 3);
        imshow("����ʶ��0-5��", frame);
        imshow("��ֵ�ָ����������ã�", mask);  // ��ʾmask�����ڹ۲�ָ�Ч��

        // ��ESC�˳�
        if (waitKey(25) == 27) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
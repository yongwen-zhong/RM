#include<iostream>
#include<opencv2/opencv.hpp>
using namespace cv;
using namespace std;
const int min_w = 80;  // ��С���
const int max_w = 300; // �����
const int min_h = 80;  // ��С�߶�
const int max_h = 300; // ���߶�

int main() {
    namedWindow("video", WINDOW_KEEPRATIO);
    VideoCapture cap("D:/cppworkapsce/cv_carrs/cars.mp4");

    // ��ȡ��Ƶ��֡��,�����ӳ�
    double fps = cap.get(CAP_PROP_FPS);
    int delay = static_cast<int>(1000.0 / fps);

    // ������˹���ģ�ͱ���������
    Ptr<BackgroundSubtractorMOG2> bgsubmog = createBackgroundSubtractorMOG2(2000, 30.0, false);

    //������̬ѧģ��
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));

    // ������ر���
    int carno = 0;
    const int line_high = 500; 
    const int offset = 30;  
    vector<Point> cars;  

    while (true) {
        Mat frame;
        cap.read(frame);

        // �Ҷ�ת��
        Mat grayFrame;
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

        // ��˹ȥ��
        Mat blurredFrame;
        GaussianBlur(grayFrame, blurredFrame, Size(5,5), 10);

        // ȥ����
        Mat mask;
        bgsubmog->apply(blurredFrame, mask);

        // ��̬ѧ����
        erode(mask, mask, kernel);

        morphologyEx(mask, mask, MORPH_OPEN, kernel);
        morphologyEx(mask, mask, MORPH_CLOSE, kernel);

        // ��ֵ�˲�
        medianBlur(mask, mask, 3);

        // ��ֵ������
        threshold(mask, mask, 127, 255, THRESH_BINARY);

        // ��������
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

        // �������������ƾ��ο�
        for (size_t i = 0; i < contours.size(); i++) {
            Rect rect = boundingRect(contours[i]);
            int x = rect.x;
            int y = rect.y;
            int w = rect.width;
            int h = rect.height;

            // �Գ����Ŀ�߽����ж�
            bool isValid = (w >= min_w) && (w <= max_w) && (h >= min_h) && (h <= max_h);
            if (!isValid) {
                continue;
            }

            rectangle(frame, rect, Scalar(0, 0, 255), 2);

            // ���㲢�洢���ĵ�
            int centerX = x + w / 2;
            int centerY = y + h / 2;
            Point cpoint(centerX, centerY);
            cars.push_back(cpoint);
        }


        // �������ĵ����飬���м���
        for (vector<Point>::iterator it = cars.begin(); it != cars.end(); ) {
            // �ж����ĵ��Ƿ�ͨ���ο��߸�������
            if (abs(it->y - line_high) <= offset) {
                carno++;
                it = cars.erase(it);  // �Ƴ��Ѽ����ĳ���
                cout << "Car count: " << carno << endl;
            }
            else {
                ++it;
            }
        }

        // �ڻ����ϻ��Ʋο���
        line(frame, Point(0, line_high), Point(frame.cols, line_high), Scalar(255, 0, 0), 2);

        // �ڻ�������ʾ�������
        string countText = "Count: " + to_string(carno);
        putText(frame, countText, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);

        // ��ʾ���ƾ��ο���ԭʼ��Ƶ֡
        imshow("video", frame);

        int key = waitKey(delay);
        if (key == 27) {  // ESC���˳�
            break;
        }
    }
    cap.release();
    destroyAllWindows();
    return 0;
}
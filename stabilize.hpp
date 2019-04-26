#ifndef ZPO_STABILIZE
#define ZPO_STABILIZE

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/video/tracking.hpp>

#define STABILIZED_TMP_FOLDER "./tmp_stabilized/"
#define TL_PI 3.1415926535

namespace tl {
    class Transform {
    private:
        double _x;
        double _y;
        double _a;
        cv::Mat _m;
    public:
        Transform(double in_x, double in_y, double in_angle);
        Transform(cv::Mat &in_matrix);
        void matrix(cv::Mat &out_matrix);
        double x();
        double y();
        double a();
    };

    cv::Mat get_transform_matrix(cv::Mat frame1, cv::Mat frame2);
    cv::Rect get_crop_for_transform(int frameWidth, int frameHeight, double xCrop, double yCrop, double angle);
    std::vector<Transform> no_motion(std::vector<Transform> &transforms, cv::Rect &crop);
    cv::Mat transform_frame(cv::Mat frame, cv::Mat transMatrix, cv::Rect crop);

    void stabilize(std::string inputPath, std::vector<std::string> imageNames);
}

#endif //ZPO_STABILIZE
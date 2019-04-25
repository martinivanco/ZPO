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
    std::vector<Transform> no_motion(std::vector<Transform> &transforms, double *maxX, double *maxY, double *maxA);
    cv::Mat transform_frame(cv::Mat frame, cv::Mat transMatrix, int maxX, int maxY, double maxA);

    void stabilize(std::string inputPath, std::vector<std::string> imageNames);
}

#endif //ZPO_STABILIZE
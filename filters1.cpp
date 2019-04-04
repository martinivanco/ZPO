#include "filters1.hpp"

// if self.contrast != 0:
//             old_mean = cv2.mean(hsv[:,:,2])[0]
//             if self.contrast < 0:
//                 c = self.contrast / 125.0 + 1.0
//                 hsv[:,:,2] = hsv[:,:,2] * c
//             if self.contrast > 0:
//                 c = self.contrast / 50.0 + 1.0
//                 hsv[:,:,2] = hsv[:,:,2] * c
//             correction = int(old_mean - cv2.mean(hsv[:,:,2])[0])
//             hsv[:,:,2] += correction

ContrastFilter::ContrastFilter(vector<string> args) {
    this->alpha = 0;
    if (args.size() < 1) {
        cerr << "No argument for contrast given. Using default value 0." << endl;
    }
    else {
        string s = args.at(0);
        try {
            this->alpha = stoi(s);
        }
        catch(const invalid_argument& ia) {
            std::cerr << "Invalid argument for contrast. It must be an integer between -100 and 100. Using default value 0." << endl;
        }
    }

    if (this->alpha < -100) {
        std::cerr << "Contrast value must be between -100 and 100. Using -100." << endl;
        this->alpha = -100;
    }
    if (this->alpha > 100) {
        std::cerr << "Contrast value must be between -100 and 100. Using 100." << endl;
        this->alpha = 100;
    }
}

int ContrastFilter::filterFrame(Mat &original, Mat &processed) {
    Mat hsv;
    cvtColor(original, hsv, COLOR_BGR2HSV);
    Mat hsv16;
    hsv.convertTo(hsv16, CV_16S);

    double c = 1.0;
    if (this->alpha < 0)
        c = this->alpha / 125.0 + 1.0;
    if (this->alpha > 0)
        c = this->alpha / 50.0 + 1.0;

    double old_sum = 0;
    double new_sum = 0;
    for (int row = 0; row < hsv16.rows; row++) {
        for (int col = 0; col < hsv16.cols; col++) {
            old_sum += hsv16.at<Vec3s>(row, col)[2];
            hsv16.at<Vec3s>(row, col)[2] = cvRound(hsv16.at<Vec3s>(row, col)[2] * c);
            new_sum += hsv16.at<Vec3s>(row, col)[2];
        }
    }

    double size = hsv16.rows * hsv16.cols;
    int correction = cvRound(old_sum / size  - new_sum / size);
    for (int row = 0; row < hsv16.rows; row++) {
        for (int col = 0; col < hsv16.cols; col++) {
            hsv16.at<Vec3s>(row, col)[2] += correction;
        }
    }

    hsv16.convertTo(hsv, CV_8U);
    cvtColor(hsv, processed, COLOR_HSV2BGR);
    return 0;
}
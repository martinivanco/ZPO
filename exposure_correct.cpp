#ifdef CUSTOM_OPENCV_LOCATION
#include <cv.hpp>
#endif //CUSTOM_OPENCV_LOCATION
#include "exposure_correct.hpp"

using namespace std;
using namespace cv;
using namespace cv::detail;

void tl::average_point() {
    cv::Mat image1 = cv::imread(inputPath + imageNames.at(0));
    cv::Mat image2 = cv::imread(inputPath + imageNames.at(1));

    imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(0), image1);

    Mat imghsv1(image1.rows, image1.cols, image1.type());
    Mat imghsv2(image2.rows, image2.cols, image2.type());
    Mat imghsv3(image2.rows, image2.cols, image2.type());

    cvtColor(image1, imghsv1, COLOR_RGB2HSV);
    cvtColor(image2, imghsv2, COLOR_RGB2HSV);

    Mat hsv1[3], hsv2[3], hsv3[3];

    split(imghsv1, hsv1);

    cv::Mat accumulator(imghsv1.rows, imghsv1.cols, CV_32F);

    for(int i=2; i < imageNames.size(); i++) {
        Mat image3 = imread(inputPath + imageNames.at(i));

        cvtColor(image3, imghsv3, COLOR_RGB2HSV);

        split(imghsv2, hsv2);
        split(imghsv3, hsv3);

        accumulateWeighted(hsv1[2], accumulator, 0.3);
        accumulateWeighted(hsv2[2], accumulator, 0.3);
        accumulateWeighted(hsv3[2], accumulator, 0.3);

        convertScaleAbs(accumulator, hsv2[2]);

        merge(hsv2, 3, imghsv2);
        cvtColor(imghsv2, image2, COLOR_HSV2RGB);

        imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i-1), image2);

        hsv1[2] = hsv2[2].clone();
        imghsv2 = imghsv3.clone();
    }
}

void tl::threshold_point(int cutoff) {

    Mat prev_img = imread(inputPath + imageNames.at(0), IMREAD_COLOR);
    cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(0), prev_img);

    cvtColor(prev_img, prev_img, COLOR_RGB2HSV);

    Mat prev_hsv[3];

    for(int n=1; n<imageNames.size(); n++){

        split(prev_img, prev_hsv);

        Mat img = imread(inputPath + imageNames.at(n), IMREAD_COLOR);

        Mat imghsv(img);
        cvtColor(img, imghsv, COLOR_RGB2HSV);

        Mat hsv[3];
        split(imghsv, hsv);

        for(int x = 0; x < img.rows; x++)
        {
            for (int y = 0; y < img.cols; y++){

                uint8_t intensity = hsv[2].at<uint8_t>(x, y);
                uint8_t prev_intensity = prev_hsv[2].at<uint8_t>(x, y);

                int diff = abs(intensity - prev_intensity);

                if(diff < cutoff){

                    if(intensity > prev_intensity){
                        if(((uint8_t) (prev_intensity + 1)) < prev_intensity) {
                            intensity = prev_intensity;
                        }
                        else {
                            intensity = prev_intensity + 1;
                        }
                    }
                    else{
                        if(((uint8_t) (prev_intensity - 1)) > prev_intensity) {
                            intensity = prev_intensity;
                        }
                        else {
                            intensity = prev_intensity - 1;
                        }
                    }

                    hsv[2].at<uint8_t>(x, y) = intensity;
                }
            }
        }

        merge(hsv, 3, imghsv);

        prev_img = imghsv.clone();

        cvtColor(imghsv, img, COLOR_HSV2RGB);

        cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(n), img);
    }
}

void tl::average_frame() {
    std::string command("mkdir -p ");
    command += EXP_CORRECTED_TMP_FOLDER;
    system(command.c_str());

    vector<double> average_exp;

    for(int i = 0; i < imageNames.size(); i++){
        cv::Mat image = cv::imread(inputPath + imageNames.at(i));

        cvtColor(image, image, COLOR_BGR2HSV);

        Mat hsv[3];
        split(image, hsv);

        double exp_sum = 0;

        for (int x = 0; x < image.rows; ++x) {
            for(int y = 0; y < image.cols; y++){
                exp_sum += (double) hsv[2].at<uint8_t>(x,y);
            }
        }

        average_exp.push_back(exp_sum / (image.rows * image.cols));
    }

    cv::Mat image = cv::imread(inputPath + imageNames.at(0));
    imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(0), image);
    image = cv::imread(inputPath + imageNames.at(1));
    imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(1), image);

    for(int i = 2; i < imageNames.size() - 2; i++){
        cv::Mat image = cv::imread(inputPath + imageNames.at(i));

        double exp_diff = ((average_exp[i-2] + average_exp[i-1] + average_exp[i] + average_exp[i+1] + average_exp[i+2]) / 5) - average_exp[i];

        Mat hsv[3];
        split(image, hsv);

        hsv[2] += exp_diff;

        merge(hsv, 3, image);

        imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i), image);
    }

    image = cv::imread(inputPath + imageNames.at(imageNames.size()-2));
    imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(imageNames.size()-2), image);
    image = cv::imread(inputPath + imageNames.at(imageNames.size()-1));
    imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(imageNames.size()-1), image);
}

void tl::exposure_correct(std::string inputPath, std::vector<std::string> imageNames) {

    tl::inputPath = inputPath;
    tl::imageNames = imageNames;

    // average_point();

    // threshold_point(20);

    average_frame();

}

#ifdef CUSTOM_OPENCV_LOCATION
#include <cv.hpp>
#endif //CUSTOM_OPENCV_LOCATION
#include "exposure_correct.hpp"

using namespace std;
using namespace cv;
using namespace cv::detail;

void tl::average_point(std::string inputPath, std::vector<std::string> imageNames) {
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

void tl::threshold_point(std::string inputPath, std::vector<std::string> imageNames, int threshold) {

    Mat prev_img = imread(inputPath + imageNames.at(0), IMREAD_COLOR);
    cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(0), prev_img);

    cvtColor(prev_img, prev_img, COLOR_BGR2HSV);

    Mat prev_hsv[3];

    for(int n=1; n<imageNames.size(); n++){

        split(prev_img, prev_hsv);

        Mat img = imread(inputPath + imageNames.at(n), IMREAD_COLOR);

        Mat imghsv(img);
        cvtColor(img, imghsv, COLOR_BGR2HSV);

        Mat hsv[3];
        split(imghsv, hsv);

        int x,y;

        for(x = 0; x < img.rows; x++)
        {
            for (y = 0; y < img.cols; y++){

                uint8_t intensity = hsv[2].at<uint8_t>(x, y);
                uint8_t prev_intensity = prev_hsv[2].at<uint8_t>(x, y);

                int offset = abs(intensity - prev_intensity);

                if(offset < threshold){
                    int diff = int(intensity) - int(prev_intensity);
                    if (abs(diff) > 10)
                        diff = diff < 0 ? -10 : 10;

                    if(intensity > prev_intensity){
                        intensity = saturate_cast<uint8_t>(prev_intensity + diff);
                    }
                    else{
                        intensity = saturate_cast<uint8_t>(prev_intensity - diff);
                    }

                    hsv[2].at<uint8_t>(x, y) = intensity;
                }
            }
        }

        merge(hsv, 3, imghsv);

        prev_img = imghsv.clone();

        cvtColor(imghsv, img, COLOR_HSV2BGR);
        cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(n), img);
    }
}

void tl::average_frame_exp(std::string inputPath, std::vector<std::string> imageNames) {
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

void tl::average_frame_hsv(std::string inputPath, std::vector<std::string> imageNames) {

    vector<double> average_exp;
    vector<double> average_hue;
    vector<double> average_sat;

    for(int i = 0; i < imageNames.size(); i++){
        cv::Mat image = cv::imread(inputPath + imageNames.at(i));

        cvtColor(image, image, COLOR_BGR2HSV);

        Mat hsv[3];
        split(image, hsv);

        double exp_sum = 0;
        double hue_sum = 0;
        double sat_sum = 0;

        for (int x = 0; x < image.rows; x++) {
            for(int y = 0; y < image.cols; y++){
                hue_sum += (double) hsv[0].at<uint8_t>(x,y);
                sat_sum += (double) hsv[1].at<uint8_t>(x,y);
                exp_sum += (double) hsv[2].at<uint8_t>(x,y);
            }
        }

        average_exp.push_back(exp_sum / (image.rows * image.cols));
        average_sat.push_back(sat_sum / (image.rows * image.cols));
        average_hue.push_back(hue_sum / (image.rows * image.cols));
    }


    cv::Mat image = cv::imread(inputPath + imageNames.at(0));
    imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(0), image);
    image = cv::imread(inputPath + imageNames.at(1));
    imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(1), image);

    for(int i = 0; i < imageNames.size(); i++){
        cv::Mat image = cv::imread(inputPath + imageNames.at(i));
        cvtColor(image, image, COLOR_BGR2HSV);

        int comp = 30;
        int lower_bound = i - comp < 0 ? 0 : i - comp;
        int upper_bound = i + comp > imageNames.size() - 1 ? imageNames.size() - 1 : i + comp;
        double exp_sum = 0;
        double hue_sum = 0;
        double sat_sum = 0;
        for (int j = lower_bound; j <= upper_bound; j++) {
            exp_sum += average_exp[j];
            hue_sum += average_hue[j];
            sat_sum += average_sat[j];
        }
        double exp_diff = (exp_sum / (upper_bound - lower_bound + 1)) - average_exp[i];
        double hue_diff = (hue_sum / (upper_bound - lower_bound + 1)) - average_hue[i];
        double sat_diff = (sat_sum / (upper_bound - lower_bound + 1)) - average_sat[i];

        Mat hsv[3];
        split(image, hsv);

        hsv[0] += hue_diff;
        hsv[1] += sat_diff;
        hsv[2] += exp_diff;

        merge(hsv, 3, image);

        cvtColor(image, image, COLOR_HSV2BGR);

        imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i), image);
    }

    image = cv::imread(inputPath + imageNames.at(imageNames.size()-2));
    imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(imageNames.size()-2), image);
    image = cv::imread(inputPath + imageNames.at(imageNames.size()-1));
    imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(imageNames.size()-1), image);
}

void tl::experiment(std::string inputPath, std::vector<std::string> imageNames) {

    vector<double> avg_b;
    vector<double> avg_g;
    vector<double> avg_r;

    Mat img0 = imread(inputPath + imageNames.at(0));

    Mat bgr0[3];
    split(img0, bgr0);

    double avg_g0 = 0, avg_b0 = 0, avg_r0 = 0;
    for(int x = 0; x < bgr0[0].rows; x++) {
        for(int y = 0; y < bgr0[0].cols; y++) {
            avg_b0 += bgr0[0].at<uint8_t>(x,y);
            avg_g0 += bgr0[1].at<uint8_t>(x,y);
            avg_r0 += bgr0[2].at<uint8_t>(x,y);
        }
    }

    avg_b0 /= (img0.rows * img0.cols);
    avg_g0 /= (img0.rows * img0.cols);
    avg_r0 /= (img0.rows * img0.cols);

    avg_b.push_back(avg_b0);
    avg_g.push_back(avg_g0);
    avg_r.push_back(avg_r0);

    Mat img1, img2;
    Mat bgr1[3], bgr2[3];

    Mat delta_b(img1.rows, img1.cols, CV_64F);
    Mat delta_g(img1.rows, img1.cols, CV_64F);
    Mat delta_r(img1.rows, img1.cols, CV_64F);

    img1 = imread(inputPath + imageNames.at(1));
    split(img1, bgr1);

    double threshold = 30;

    for(int i = 1; i < imageNames.size()-1; i++) {

        img2 = imread(inputPath + imageNames.at(i+1));

        split(img2, bgr2);

        delta_b = bgr1[0] - bgr2[0];
        delta_g = bgr1[1] - bgr2[1];
        delta_r = bgr1[2] - bgr2[2];

        double mean_delta_b = 0;
        double mean_delta_g = 0;
        double mean_delta_r = 0;

        for(int x = 0; x < delta_b.rows; x++){
            for(int y = 0; y < delta_b.cols; y++){

                if(abs(delta_b.at<double>(x,y)) > threshold){
                    delta_b.at<double>(x,y) = 0;
                }

                if(abs(delta_g.at<double>(x,y)) > threshold){
                    delta_g.at<double>(x,y) = 0;
                }

                if(abs(delta_r.at<double>(x,y)) > threshold){
                    delta_r.at<double>(x,y) = 0;
                }

                mean_delta_b += delta_b.at<double>(x,y);
                mean_delta_g += delta_g.at<double>(x,y);
                mean_delta_r += delta_r.at<double>(x,y);
            }
        }

        mean_delta_b /= (delta_b.cols * delta_b.rows);
        mean_delta_g /= (delta_b.cols * delta_b.rows);
        mean_delta_r /= (delta_b.cols * delta_b.rows);

        avg_b.push_back((avg_b[i - 1]) + mean_delta_b);
        avg_g.push_back((avg_g[i - 1]) + mean_delta_g);
        avg_r.push_back((avg_r[i - 1]) + mean_delta_r);

        bgr1[0] = bgr2[0].clone();
        bgr1[1] = bgr2[1].clone();
        bgr1[2] = bgr2[2].clone();
    }


    for(int i = 0; i < imageNames.size(); i++){

        int window = 30;
        int lower_bound = i - window < 0 ? 0 : i - window;
        int upper_bound = i + window > imageNames.size() - 1 ? imageNames.size() - 1 : i + window;

        double sum_b = 0;
        double sum_g = 0;
        double sum_r = 0;

        for (int j = lower_bound; j <= upper_bound; j++) {
            sum_b += avg_b[j];
            sum_g += avg_g[j];
            sum_r += avg_r[j];
        }

        double diff_b = (sum_b / (upper_bound - lower_bound + 1));
        double diff_g = (sum_g / (upper_bound - lower_bound + 1));
        double diff_r = (sum_r / (upper_bound - lower_bound + 1));

        cv::Mat image = cv::imread(inputPath + imageNames.at(i));

        Mat bgr[3];
        split(image, bgr);

        double img_avg_b = 0;
        double img_avg_g = 0;
        double img_avg_r = 0;

        for(int x = 0; x < image.rows; x++){
            for(int y = 0; y < image.cols; y++){
                img_avg_b += bgr[0].at<uint8_t>(x,y);
                img_avg_g += bgr[1].at<uint8_t>(x,y);
                img_avg_r += bgr[2].at<uint8_t>(x,y);
            }
        }

        img_avg_b /= (image.rows * image.cols);
        img_avg_g /= (image.rows * image.cols);
        img_avg_r /= (image.rows * image.cols);

        diff_b -= img_avg_b;
        diff_g -= img_avg_g;
        diff_r -= img_avg_r;

        for (int x = 0; x < image.rows; x++) {
            for (int y = 0; y < image.cols; y++) {
                if(bgr[0].at<uint8_t>(x, y) > 20 and bgr[1].at<uint8_t>(x, y) > 20 and bgr[2].at<uint8_t>(x, y) > 20){
                    bgr[0].at<uint8_t>(x, y) = saturate_cast<uint8_t>(double(bgr[0].at<uint8_t>(x, y)) + diff_b);
                    bgr[1].at<uint8_t>(x, y) = saturate_cast<uint8_t>(double(bgr[1].at<uint8_t>(x, y)) + diff_g);
                    bgr[2].at<uint8_t>(x, y) = saturate_cast<uint8_t>(double(bgr[2].at<uint8_t>(x, y)) + diff_r);
                }
            }
        }

        merge(bgr, 3, image);

        imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i), image);
    }


}

void tl::exposure_correct(std::string inputPath, std::vector<std::string> imageNames) {

    std::string command("mkdir -p ");
    command += EXP_CORRECTED_TMP_FOLDER;
    system(command.c_str());

    // average_point(inputPath, imageNames);

    // threshold_point(inputPath, imageNames, 20);

    // average_frame_exp(inputPath, imageNames);

    // average_frame_hsv(inputPath, imageNames);

    experiment(inputPath, imageNames);
}

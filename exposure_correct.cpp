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
        // cv::resize(img, img, cv::Size(1200, 800));
        cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(n), img);
    }


    /*
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
    }*/
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
        std::cout << "Diffs: " << exp_diff << " " << hue_diff << " " << sat_diff << std::endl;

        Mat hsv[3];
        split(image, hsv);

        std::cout << "Check: " << unsigned(hsv[2].at<uint8_t>(24,26)) << " -> ";
        // hsv[0] += hue_diff;
        // hsv[1] += sat_diff;
        hsv[2] += 100;
        // hsv[2].convertTo(hsv[2], -1, 1, exp_diff);
        std::cout << unsigned(hsv[2].at<uint8_t>(24,26)) << std::endl;

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

    vector<double> avg_h;
    vector<double> avg_s;
    vector<double> avg_v;

    Mat img0 = imread(inputPath + imageNames.at(0));
    //cvtColor(img0, img0, COLOR_BGR2HSV);

    Mat hsv0[3];
    split(img0, hsv0);

    double avg_h0 = 0, avg_s0 = 0, avg_v0 = 0;
    for(int x = 0; x < hsv0[0].rows; x++) {
        for(int y = 0; y < hsv0[0].cols; y++) {
            avg_h0 += hsv0[0].at<uint8_t>(x,y);
            avg_s0 += hsv0[1].at<uint8_t>(x,y);
            avg_v0 += hsv0[2].at<uint8_t>(x,y);
        }
    }

    avg_h0 /= (img0.rows * img0.cols);
    avg_s0 /= (img0.rows * img0.cols);
    avg_v0 /= (img0.rows * img0.cols);

    avg_h.push_back(avg_h0);
    avg_s.push_back(avg_s0);
    avg_v.push_back(avg_v0);

    Mat img1, img2;
    Mat hsv1[3], hsv2[3];

    Mat delta_h(img1.rows, img1.cols, CV_64F);
    Mat delta_s(img1.rows, img1.cols, CV_64F);
    Mat delta_v(img1.rows, img1.cols, CV_64F);

    img1 = imread(inputPath + imageNames.at(1));
    //cvtColor(img1, img1, COLOR_BGR2HSV);
    split(img1, hsv1);

    // TODO!!!!!!!!!!!
    double threshold = 30;

    for(int i = 1; i < imageNames.size()-1; i++) {

        img2 = imread(inputPath + imageNames.at(i+1));
        //cvtColor(img2, img2, COLOR_BGR2HSV);

        split(img2, hsv2);

        delta_h = hsv1[0] - hsv2[0];
        delta_s = hsv1[1] - hsv2[1];
        delta_v = hsv1[2] - hsv2[2];

        double mean_delta_h = 0;
        double mean_delta_s = 0;
        double mean_delta_v = 0;

        for(int x = 0; x < delta_h.rows; x++){
            for(int y = 0; y < delta_h.cols; y++){

                if(abs(delta_h.at<double>(x,y)) > threshold){
                    delta_h.at<double>(x,y) = 0;
                }

                if(abs(delta_s.at<double>(x,y)) > threshold){
                    delta_s.at<double>(x,y) = 0;
                }

                if(abs(delta_v.at<double>(x,y)) > threshold){
                    delta_v.at<double>(x,y) = 0;
                }

                mean_delta_h += delta_h.at<double>(x,y);
                mean_delta_s += delta_s.at<double>(x,y);
                mean_delta_v += delta_v.at<double>(x,y);
            }
        }

        mean_delta_h /= (delta_h.cols * delta_h.rows);
        mean_delta_s /= (delta_h.cols * delta_h.rows);
        mean_delta_v /= (delta_h.cols * delta_h.rows);

        avg_h.push_back((avg_h[i - 1]) + mean_delta_h);
        avg_s.push_back((avg_s[i - 1]) + mean_delta_s);
        avg_v.push_back((avg_v[i - 1]) + mean_delta_v);

        hsv1[0] = hsv2[0].clone();
        hsv1[1] = hsv2[1].clone();
        hsv1[2] = hsv2[2].clone();
    }


    for(int i = 0; i < imageNames.size(); i++){

        int window = 30;
        int lower_bound = i - window < 0 ? 0 : i - window;
        int upper_bound = i + window > imageNames.size() - 1 ? imageNames.size() - 1 : i + window;

        double sum_h = 0;
        double sum_s = 0;
        double sum_v = 0;

        for (int j = lower_bound; j <= upper_bound; j++) {
            sum_h += avg_h[j];
            sum_s += avg_s[j];
            sum_v += avg_v[j];
        }

        double diff_h = (sum_h / (upper_bound - lower_bound + 1));
        double diff_s = (sum_s / (upper_bound - lower_bound + 1));
        double diff_v = (sum_v / (upper_bound - lower_bound + 1));

        cv::Mat image = cv::imread(inputPath + imageNames.at(i));
        //cvtColor(image, image, COLOR_BGR2HSV);

        Mat hsv[3];
        split(image, hsv);

        double img_avg_h = 0;
        double img_avg_s = 0;
        double img_avg_v = 0;

        for(int x = 0; x < image.rows; x++){
            for(int y = 0; y < image.cols; y++){
                img_avg_h += hsv[0].at<uint8_t>(x,y);
                img_avg_s += hsv[1].at<uint8_t>(x,y);
                img_avg_v += hsv[2].at<uint8_t>(x,y);
            }
        }

        img_avg_h /= (image.rows * image.cols);
        img_avg_s /= (image.rows * image.cols);
        img_avg_v /= (image.rows * image.cols);

        diff_h -= img_avg_h;
        diff_s -= img_avg_s;
        diff_v -= img_avg_v;

        std::cout << "Check: " << unsigned(hsv[2].at<uint8_t>(24,26)) << " -> ";

        for (int x = 0; x < image.rows; x++) {
            for (int y = 0; y < image.cols; y++) {
                if(hsv[0].at<uint8_t>(x, y) > 20 and hsv[0].at<uint8_t>(x, y) > 20 and hsv[0].at<uint8_t>(x, y) > 20){
                    hsv[0].at<uint8_t>(x, y) = saturate_cast<uint8_t>(hsv[0].at<uint8_t>(x, y) + diff_h);
                    hsv[1].at<uint8_t>(x, y) = saturate_cast<uint8_t>(hsv[1].at<uint8_t>(x, y) + diff_s);
                    hsv[2].at<uint8_t>(x, y) = saturate_cast<uint8_t>(hsv[2].at<uint8_t>(x, y) + diff_v);
                }
            }
        }

        /*
        hsv[0].convertTo(hsv[0], -1, 1, diff_h);
        hsv[1].convertTo(hsv[1], -1, 1, diff_s);
        hsv[2].convertTo(hsv[2], -1, 1, diff_v);
         */

        std::cout << unsigned(hsv[2].at<uint8_t>(24,26)) << std::endl;

        merge(hsv, 3, image);

        //cvtColor(image, image, COLOR_HSV2BGR);

        imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i), image);
    }


}

void tl::exposure_correct(std::string inputPath, std::vector<std::string> imageNames) {

    std::string command("mkdir -p ");
    command += EXP_CORRECTED_TMP_FOLDER;
    system(command.c_str());

    // average_point(inputPath, imageNames);

    //threshold_point(inputPath, imageNames, 20);

    //average_frame_exp(inputPath, imageNames);

    //average_frame_hsv(inputPath, imageNames);

    experiment(inputPath, imageNames);
}

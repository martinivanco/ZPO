#ifdef CUSTOM_OPENCV_LOCATION
#include <cv.hpp>
#endif //CUSTOM_OPENCV_LOCATION
#include "exposure_correct.hpp"

using namespace std;
using namespace cv;
using namespace cv::detail;

void tl::exposure_correct(std::string inputPath, std::vector<std::string> imageNames) {
    std::string command("mkdir -p ");
    command += EXP_CORRECTED_TMP_FOLDER;
    system(command.c_str());

    /*
    *    ======================= EXPOSURE COMPENSATOR START ======================
    *    Probably we can not use this for our problem
    */

    /*
    Ptr<ExposureCompensator> compensator = ExposureCompensator::createDefault(ExposureCompensator::GAIN);

    vector<Point> corners(imageNames.size());
    vector<UMat> uimages(imageNames.size());
    vector<std::pair<UMat, uchar>> masks(imageNames.size());

    for(int i=0; i<imageNames.size(); i++){
        corners[i] = Point(0,0);
        uimages[i] = imread(inputPath + imageNames.at(i), IMREAD_COLOR).getUMat(ACCESS_RW);
        masks[i].first = UMat::zeros(uimages.at(i).rows, uimages.at(i).cols, CV_8U);
        masks[i].second = 0;//255;
    }

    compensator->feed(corners, uimages, masks);

    UMat output(uimages.at(0).rows, uimages.at(0).cols, uimages.at(0).type());
    Mat mask = Mat::zeros(uimages[0].rows,uimages[0].cols, CV_8U);

    for(int i=0; i < imageNames.size(); i++) {
        compensator->apply(i,Point(0,0), output, mask);
        cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i), output.getMat(ACCESS_READ));// uimages.at(i).getMat(ACCESS_READ));
    } */

    /*
    *    ======================= EXPOSURE COMPENSATOR END ======================
    */


    // vector<double> average_exp;
    // vector<double> average_hue;
    // vector<double> average_sat;

    // for(int i = 0; i < imageNames.size(); i++){
    //     cv::Mat image = cv::imread(inputPath + imageNames.at(i));

    //     cvtColor(image, image, COLOR_BGR2HSV);

    //     Mat hsv[3];
    //     split(image, hsv);

    //     double exp_sum = 0;
    //     double hue_sum = 0;
    //     double sat_sum = 0;

    //     for (int x = 0; x < image.rows; x++) {
    //         for(int y = 0; y < image.cols; y++){
    //             hue_sum += (double) hsv[0].at<uint8_t>(x,y);
    //             sat_sum += (double) hsv[1].at<uint8_t>(x,y);
    //             exp_sum += (double) hsv[2].at<uint8_t>(x,y);
    //         }
    //     }

    //     average_exp.push_back(exp_sum / (image.rows * image.cols));
    //     average_sat.push_back(sat_sum / (image.rows * image.cols));
    //     average_hue.push_back(hue_sum / (image.rows * image.cols));
    // }

    // // cv::Mat image = cv::imread(inputPath + imageNames.at(0));
    // // imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(0), image);
    // // image = cv::imread(inputPath + imageNames.at(1));
    // // imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(1), image);

    // for(int i = 0; i < imageNames.size(); i++){
    //     cv::Mat image = cv::imread(inputPath + imageNames.at(i));
    //     cvtColor(image, image, COLOR_BGR2HSV);

    //     int comp = 30;
    //     int lower_bound = i - comp < 0 ? 0 : i - comp;
    //     int upper_bound = i + comp > imageNames.size() - 1 ? imageNames.size() - 1 : i + comp;
    //     double exp_sum = 0;
    //     double hue_sum = 0;
    //     double sat_sum = 0;
    //     for (int j = lower_bound; j <= upper_bound; j++) {
    //         exp_sum += average_exp[j];
    //         hue_sum += average_hue[j];
    //         sat_sum += average_sat[j];
    //     }
    //     double exp_diff = (exp_sum / (upper_bound - lower_bound + 1)) - average_exp[i];
    //     double hue_diff = (hue_sum / (upper_bound - lower_bound + 1)) - average_hue[i];
    //     double sat_diff = (sat_sum / (upper_bound - lower_bound + 1)) - average_sat[i];
    //     std::cout << "Diffs: " << exp_diff << " " << hue_diff << " " << sat_diff << std::endl;

    //     Mat hsv[3];
    //     split(image, hsv);

    //     std::cout << "Check: " << unsigned(hsv[2].at<uint8_t>(24,26)) << " -> ";
    //     // hsv[0] += hue_diff;
    //     // hsv[1] += sat_diff;
    //     hsv[2] += 100;
    //     // hsv[2].convertTo(hsv[2], -1, 1, exp_diff);
    //     std::cout << unsigned(hsv[2].at<uint8_t>(24,26)) << std::endl;

    //     merge(hsv, 3, image);

    //     cvtColor(image, image, COLOR_HSV2BGR);

    //     imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i), image);
    // }

    // image = cv::imread(inputPath + imageNames.at(imageNames.size()-2));
    // imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(imageNames.size()-2), image);
    // image = cv::imread(inputPath + imageNames.at(imageNames.size()-1));
    // imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(imageNames.size()-1), image);


    /*
    *    =================== 3 FRAMES ACCUMULATED START ==================
    */
    ///TODO: Look at this!

    /*

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
    }*/
    

    /*
    *    =================== 3 FRAMES ACCUMULATED END ==================
    */


    /*
    *    ======================= BORDEL START ======================
    */

    //cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i), output);
    //cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i), output)

    /*
    for(int i = 0; i < imageNames.size(); i++) {
        cv::Mat image = cv::imread(inputPath + imageNames.at(i));
        
        // TODO PROCESSING

        cv::Mat output = image;
        cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i), output);
    }*/

    /*for (int e = 0; e < imageNames.size(); e++) {
        printf("file %d name: %s\n", e, imageNames[e].data());
    }*/

    /*
    *    ======================= BORDEL END ======================
    */


    /*
    *    ==================== EXPOSURE (2 FRAMES) START ======================
    */

    
    Mat prev_img = imread(inputPath + imageNames.at(0), IMREAD_COLOR);
    cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(0), prev_img);

    cvtColor(prev_img, prev_img, COLOR_BGR2HSV);

    Mat prev_hsv[3];

    int strengthcutoff = 30;

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

                int strength = abs(intensity - prev_intensity);

                if(strength < strengthcutoff){
                    int diff = int(intensity) - int(prev_intensity);
                    // std::cout << diff << std::endl;
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
    *    ==================== EXPOSURE (2 FRAMES) END ======================
    */
}

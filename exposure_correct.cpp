#include <cv.hpp>
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
    */

    /* NOT WORKING
    Ptr<ExposureCompensator> compensator = ExposureCompensator::createDefault(ExposureCompensator::GAIN);
    vector<Point> corners(imageNames.size());
    vector<Mat> images(imageNames.size());
    vector<UMat> uimages(imageNames.size());
    vector<UMat> masks(imageNames.size());

    for(int i=0; i<imageNames.size(); i++){
        corners[i] = Point(0,0);
        images[i] = imread(inputPath + imageNames[i], IMREAD_COLOR);
        uimages[i] = images[i].getUMat(ACCESS_RW);
        masks[i] = UMat::ones(uimages[i].rows, uimages[i].cols, CV_8U);
    }

    compensator->feed(corners, uimages, masks);

    Mat mask = Mat::ones(images[0].rows,images[0].cols, CV_8U);

    for(int i=0; i < imageNames.size(); i++) {
        compensator->apply(i,Point(0,0),uimages[i], mask);
        images[i] = uimages[i].getMat(ACCESS_RW);
        cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i), images[i]);
    } */

    /*
    *    ======================= EXPOSURE COMPENSATOR END ======================
    */


    /*
    *    ======================= BORDEL START ======================
    */

    /*
    cv::Mat image1 = cv::imread(inputPath + imageNames.at(0));
    cv::Mat image2 = cv::imread(inputPath + imageNames.at(1));

    cv::Mat output;
    Mat frame;
    Mat gray = Mat::zeros(image1.rows, image1.cols, CV_32FC3);

    cvtColor(image2 ,gray , CV_8UC3, 0);
    accumulateWeighted(image1, gray, 0.005);

    while(1){
        imshow("gray", gray);
        imshow("acc", image1);
        waitKey(1); //don't know why I need it but without it the windows freezes
    }*/

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

    cvtColor(prev_img, prev_img, COLOR_RGB2HSV);

    int strengthcutoff = 20;

    for(int n=1; n<imageNames.size(); n++){

        Mat img = imread(inputPath + imageNames.at(n), IMREAD_COLOR);

        Mat imghsv(img);
        cvtColor(img, imghsv, COLOR_RGB2HSV);

        int x,y;

        for(x = 0; x < img.rows; x++)
        {
            for (y = 0; y < img.cols; y++){ ;
                Vec3b intensity = imghsv.at<Vec3b>(x, y);
                Vec3b prev_intensity = prev_img.at<Vec3b>(x, y);

                int strength = abs(intensity.val[3] - prev_intensity.val[3]);

                if(strength < strengthcutoff){

                    // TODO: 0 -> 255 -> 0

                    if(intensity.val[3] > prev_intensity.val[3]){
                        intensity.val[3] = prev_intensity.val[3] + 1;
                    }
                    else{
                        intensity.val[3] = prev_intensity.val[3] - 1;
                    }

                    imghsv.at<Vec3b>(x, y) = intensity;
                }
            }
        }

        prev_img = imghsv.clone();

        cvtColor(imghsv, img, COLOR_HSV2RGB);

        cv::imwrite(EXP_CORRECTED_TMP_FOLDER + imageNames.at(n), img);
    }

    /*
    *    ==================== EXPOSURE (2 FRAMES) END ======================
    */
}

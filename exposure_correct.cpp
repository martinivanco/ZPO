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

    /*
    *    =================== 3 FRAMES ACCUMULATED START ==================
    */
    ///TODO: Look at this!


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

    cvtColor(prev_img, prev_img, COLOR_RGB2HSV);

    printf("Here %d", 10);
    Mat prev_hsv[3];

    int strengthcutoff = 20;

    for(int n=1; n<imageNames.size(); n++){

        split(prev_img, prev_hsv);

        Mat img = imread(inputPath + imageNames.at(n), IMREAD_COLOR);

        Mat imghsv(img);
        cvtColor(img, imghsv, COLOR_RGB2HSV);

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

    /*
    *    ==================== EXPOSURE (2 FRAMES) END ======================
    */
}

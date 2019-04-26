#include "stabilize.hpp"

tl::Transform::Transform(double in_x, double in_y, double in_angle) {
    _x = in_x;
    _y = in_y;
    _a = in_angle;
    _m = cv::Mat(2, 3, CV_64F);
    _m.at<double>(0,0) = cos(_a);
    _m.at<double>(0,1) = -sin(_a);
    _m.at<double>(1,0) = sin(_a);
    _m.at<double>(1,1) = cos(_a);
    _m.at<double>(0,2) = _x;
    _m.at<double>(1,2) = _y;
}
tl::Transform::Transform(cv::Mat &in_matrix) {
    in_matrix.copyTo(_m);
    _x = _m.at<double>(0,2);
    _y = _m.at<double>(1,2);
    _a = atan2(_m.at<double>(1,0), _m.at<double>(0,0));
}
void tl::Transform::matrix(cv::Mat &out_matrix) {
    _m.copyTo(out_matrix);
}
double tl::Transform::x() {
    return _x;
}
double tl::Transform::y() {
    return _y;
}
double tl::Transform::a() {
    return _a;
}

cv::Mat tl::get_transform_matrix(cv::Mat frame1, cv::Mat frame2) {
    cv::Mat f1, f2; // grayscale versions of frames
    std::vector<cv::Point2f> p1, p2; // feature points of frames

    // convert both frames to grayscale
    cv::cvtColor(frame1, f1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(frame2, f2, cv::COLOR_BGR2GRAY);

    // get feature points from first frame
    cv::goodFeaturesToTrack(f1, p1, 200, 0.01, 30); // TODO check values

    // find those points on second frame
    std::vector<uchar> status;
    std::vector<float> error;
    calcOpticalFlowPyrLK(f1, f2, p1, p2, status, error);
    
    // get rid of invalid points
    auto i1 = p1.begin(); 
    auto i2 = p2.begin(); 
    for (int k = 0; k < status.size(); k++) {
        if(status.at(k)) {
            i1++; 
            i2++; 
        }
        else {
            i1 = p1.erase(i1);
            i2 = p2.erase(i2);
        }
    }
        
    // calculate transformation matrix
    return cv::estimateAffinePartial2D(p1, p2);
}

cv::Rect tl::get_crop_for_transform(int frameWidth, int frameHeight, double xCrop, double yCrop, double angle) {
    // create rectangle without crop
    cv::Rect roi(0, 0, frameWidth, frameHeight);

    // crop to remove borders due to translation
    roi.x += (int) round(xCrop);
    roi.y += (int) round(yCrop);
    roi.width -= 2 * roi.x;
    roi.height -= 2 * roi.y;

    // crop to remove borders due to rotation
    int quadrant = ((int) floor(angle / (TL_PI / 2))) & 3;
    double signAlpha = (quadrant & 1) == 0 ? angle : TL_PI - angle;
    double alpha = fmod(fmod(signAlpha, TL_PI) + TL_PI, TL_PI);
    double bbw = roi.width * cos(alpha) + roi.height * sin(alpha);
    double bbh = roi.width * sin(alpha) + roi.height * cos(alpha);
    double gamma = roi.width < roi.height ? atan2(bbw, bbh) : atan2(bbh, bbw);
    double delta = TL_PI - alpha - gamma;
    int length = roi.width < roi.height ? roi.height : roi.width;
    double d = length * cos(alpha);
    double a = d * sin(alpha) / sin(delta);
    int y = (int) ceil(a * cos(gamma));
    int x = (int) ceil(y * tan(gamma));

    roi.x += x;
    roi.y += y;
    roi.width -= 2 * x;
    roi.height -= 2 * y;

    // crop to 16:9 ratio
    if (double(roi.width) / roi.height > 16.0 / 9.0) {
        int newWidth = (int) round(16.0 / 9.0 * roi.width);
        roi.x += (roi.width - newWidth) / 2;
        roi.width = newWidth;
    }
    else {
        int newHeight = (int) round(roi.height / (16.0 / 9.0));
        roi.y += (roi.height - newHeight) / 2;
        roi.height = newHeight;
    }

    return roi;
}

std::vector<tl::Transform> tl::no_motion(std::vector<tl::Transform> &transforms, cv::Rect &crop) {
    // calculate trajectory
    std::vector<tl::Transform> trajectory;
    double tranSumX = 0, tranSumY = 0, tranSumA = 0;
    double trajSumX = 0, trajSumY = 0, trajSumA = 0;

    for (int i = 0; i < transforms.size(); i++) {
        tranSumX += transforms[i].x();
        tranSumY += transforms[i].y();
        tranSumA += transforms[i].a();
        trajectory.push_back(tl::Transform(tranSumX, tranSumY, tranSumA));
        trajSumX += tranSumX;
        trajSumY += tranSumY;
        trajSumA += tranSumA;
    }

    // center of trajectory
    double cX = trajSumX / transforms.size();
    double cY = trajSumY / transforms.size();
    double cA = trajSumA / transforms.size();

    // calculate stabilization transformations
    std::vector<tl::Transform> stableTransforms;
    double maxX = 0, maxY = 0, maxA = 1;
    
    for (int i = 0; i < transforms.size(); i++) {
        double sx = cX - trajectory.at(i).x() + transforms.at(i).x();
        maxX = sx > maxX ? sx : maxX;
        double sy = cY - trajectory.at(i).y() + transforms.at(i).y();
        maxY = sy > maxY ? sy : maxY;
        double sa = cA - trajectory.at(i).a() + transforms.at(i).a();
        maxA = abs(cos(sa)) < maxA ? abs(cos(sa)) : maxA;
        stableTransforms.push_back(tl::Transform(sx, sy, sa));
    }

    crop = tl::get_crop_for_transform(crop.width, crop.height, maxX, maxY, acos(maxA));
    return stableTransforms;
}

cv::Mat tl::transform_frame(cv::Mat frame, cv::Mat transMatrix, cv::Rect crop) {
    // transform frame
    cv::Mat tFrame;
    cv::warpAffine(frame, tFrame, transMatrix, frame.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, 255);

    // crop and resize to FullHD
    cv::Mat cFrame = tFrame(crop);
    if(cFrame.cols > 1920) 
        cv::resize(cFrame, cFrame, cv::Size(1920, 1080));

    return cFrame;
}

void tl::stabilize(std::string inputPath, std::vector<std::string> imageNames) {
    // create temporary folder
    std::string command("mkdir -p ");
    command += STABILIZED_TMP_FOLDER;
    system(command.c_str());
    
    cv::Mat pFrame, cFrame; // previous and current frame
    std::vector<tl::Transform> transforms; // affine transformations (movement) between frames
    std::vector<tl::Transform> stableTransforms; // affine transformations that stabilize frames
    cv::Mat tmpMatrix; // help temporary transformation matrix in case new one fails to be found

    // inter-frame movement calculation
    pFrame = cv::imread(inputPath + imageNames.at(0));
    for (int i = 1; i < imageNames.size(); i++) {
        cFrame = cv::imread(inputPath + imageNames.at(i));
        
        cv::Mat transMatrix = tl::get_transform_matrix(pFrame, cFrame);
        if (transMatrix.data != NULL) {
            transforms.push_back(tl::Transform(transMatrix));
            transMatrix.copyTo(tmpMatrix);
        }
        else {
            transforms.push_back(tl::Transform(tmpMatrix));
        }

        cFrame.copyTo(pFrame);
    }

    // get transformations to eliminate movement
    cv::Rect crop(0, 0, cFrame.cols, cFrame.rows);
    stableTransforms = tl::no_motion(transforms, crop);

    cv::Mat oFrame, sFrame; // original and stabilized frame
    cv::Mat transMatrix(2, 3, CV_64F); // transformation matrix that stabilizes frame

    // stabilize frames
    for (int i = 0; i < imageNames.size(); i++) {
        oFrame = cv::imread(inputPath + imageNames.at(i));
        if (i < stableTransforms.size())
            stableTransforms.at(i).matrix(transMatrix);
        sFrame = tl::transform_frame(oFrame, transMatrix, crop);
        cv::imwrite(STABILIZED_TMP_FOLDER + imageNames.at(i), sFrame);
    }
}
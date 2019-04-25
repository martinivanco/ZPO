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

std::vector<tl::Transform> tl::no_motion(std::vector<tl::Transform> &transforms, double *maxX, double *maxY, double *maxA) {
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
    
    for (int i = 0; i < transforms.size(); i++) {
        double sx = cX - trajectory.at(i).x() + transforms.at(i).x();
        *maxX = sx > *maxX ? sx : *maxX;
        double sy = cY - trajectory.at(i).y() + transforms.at(i).y();
        *maxY = sy > *maxY ? sy : *maxY;
        double sa = cA - trajectory.at(i).a() + transforms.at(i).a();
        *maxA = abs(cos(sa)) < *maxA ? abs(cos(sa)) : *maxA;
        stableTransforms.push_back(tl::Transform(sx, sy, sa));
    }

    *maxA = acos(*maxA);
    return stableTransforms;
}

cv::Mat tl::transform_frame(cv::Mat frame, cv::Mat transMatrix, int maxX, int maxY, double maxA) {
    // transform frame
    cv::Mat tFrame;
    cv::warpAffine(frame, tFrame, transMatrix, frame.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, 255);

    // crop to remove borders due to translation
    cv::Rect roi;
    roi.width = tFrame.cols - 2 * maxX;
    roi.height = tFrame.rows - 2 * maxY;
    roi.x = maxX;
    roi.y = maxY;
    cv::Mat tmp = tFrame(roi);
    tmp.copyTo(tFrame);

    // crop to remove borders due to rotation

//     function getCropCoordinates(angleInRadians, imageDimensions) {
//     var ang = angleInRadians;
//     var img = imageDimensions;

//     var quadrant = Math.floor(ang / (Math.PI / 2)) & 3;
//     var sign_alpha = (quadrant & 1) === 0 ? ang : Math.PI - ang;
//     var alpha = (sign_alpha % Math.PI + Math.PI) % Math.PI;

//     var bb = {
//         w: img.w * Math.cos(alpha) + img.h * Math.sin(alpha),
//         h: img.w * Math.sin(alpha) + img.h * Math.cos(alpha)
//     };

//     var gamma = img.w < img.h ? Math.atan2(bb.w, bb.h) : Math.atan2(bb.h, bb.w);

//     var delta = Math.PI - alpha - gamma;

//     var length = img.w < img.h ? img.h : img.w;
//     var d = length * Math.cos(alpha);
//     var a = d * Math.sin(alpha) / Math.sin(delta);

//     var y = a * Math.cos(gamma);
//     var x = y * Math.tan(gamma);

//     return {
//         x: x,
//         y: y,
//         w: bb.w - 2 * x,
//         h: bb.h - 2 * y
//     };
// }

    int quadrant = ((int) floor(maxA / (3.1415926535 / 2))) & 3;
    double signAlpha = (quadrant & 1) == 0 ? maxA : 3.1415926535 - maxA;
    double alpha = fmod(fmod(signAlpha, 3.1415926535) + 3.1415926535, 3.1415926535);
    double bbw = tmp.cols * cos(alpha) + tmp.rows * sin(alpha);
    double bbh = tmp.cols * sin(alpha) + tmp.rows * cos(alpha);
    double gamma = tmp.cols < tmp.rows ? atan2(bbw, bbh) : atan2(bbh, bbw);
    double delta = 3.1415926535 - alpha - gamma;
    int length = tmp.cols < tmp.rows ? tmp.rows : tmp.cols;
    double d = length * cos(alpha);
    double a = d * sin(alpha) / sin(delta);
    double y = a * cos(gamma);
    double x = y * tan(gamma);

    std::cout << "tmp_w: " << tmp.cols << std::endl;
    std::cout << "tmp_h: " << tmp.rows << std::endl;
    std::cout << "a: " << maxA * 180.0 / 3.1415926535 << std::endl;
    std::cout << "x: " << x << std::endl;
    std::cout << "y: " << y << std::endl;
    std::cout << "w: " << bbw - 2 * x << std::endl;
    std::cout << "h: " << bbh - 2 * y << std::endl << std::endl;


    // cv::Mat rotMatrix(3, 3, CV_64F, cv::Scalar(0));
    // rotMatrix.at<double>(0,0) = cos(maxA);
    // rotMatrix.at<double>(0,1) = -sin(maxA);
    // rotMatrix.at<double>(1,0) = sin(maxA);
    // rotMatrix.at<double>(1,1) = cos(maxA);
    // rotMatrix.at<double>(2,2) = 1;

    // double rx = tmp.cols / 2.0;
    // double ry = tmp.rows / 2.0;
    // cv::Mat p1(3, 1, CV_64F, cv::Scalar(0));
    // p1.at<double>(0, 0) = -rx;
    // p1.at<double>(1, 0) = ry;
    // p1.at<double>(2, 0) = 1;
    // cv::Mat p2(3, 1, CV_64F, cv::Scalar(0));
    // p2.at<double>(0, 0) = rx;
    // p2.at<double>(1, 0) = ry;
    // p2.at<double>(2, 0) = 1;
    // cv::Mat p3(3, 1, CV_64F, cv::Scalar(0));
    // p3.at<double>(0, 0) = rx;
    // p3.at<double>(1, 0) = -ry;
    // p3.at<double>(2, 0) = 1;

    // cv::Mat rp1 = rotMatrix * p1;
    // cv::Mat rp2 = rotMatrix * p2;
    // cv::Mat rp3 = rotMatrix * p3;
    // // std::cout << rotMatrix << std::endl << std::endl;
    // // std::cout << p1 << std::endl << std::endl;
    // // std::cout << rp1 << std::endl << std::endl;

    // double a1 = rp2.at<double>(0, 0) - rp1.at<double>(0, 0);
    // double b1 = rp2.at<double>(1, 0) - rp1.at<double>(1, 0);
    // double c1 = rp1.at<double>(0, 0) * a1 + rp1.at<double>(1, 0) * b1;
    // double d1 = -b1;
    // double e1 = a1;
    // double f1 = 0;
    // cv::Point2d point1((b1*f1 - e1*c1) / (b1*d1 - e1*a1), (a1*f1 - d1*c1) / (a1*e1 - d1*b1));
    // std::cout << point1 << std::endl;

    // // std::cout << a1 << std::endl;
    // // std::cout << b1 << std::endl;
    // // std::cout << c1 << std::endl << std::endl;
    // // cv::Point2d point1(2 * c1 / a1, 2 * c1 / b1);
    // // std::cout << point1 << std::endl;

    // double a2 = rp3.at<double>(0, 0) - rp2.at<double>(0, 0);
    // double b2 = rp3.at<double>(1, 0) - rp2.at<double>(1, 0);
    // double c2 = rp2.at<double>(0, 0) * a2 + rp2.at<double>(1, 0) * b2;
    // double d2 = -b2;
    // double e2 = a2;
    // double f2 = 0;
    // cv::Point2d point2((b2*f2 - e2*c2) / (b2*d2 - e2*a2), (a2*f2 - d2*c2) / (a2*e2 - d2*b2));
    // std::cout << point2 << std::endl;

    // // cv::Point2d point2(2 * c2 / a2, 2 * c2 / b2);
    // // std::cout << point2 << std::endl;


    // crop and resize to FullHD
    if (double(tFrame.cols) / tFrame.rows > 16.0 / 9.0) {
        roi.width = (int) round(16.0 / 9.0 * tFrame.rows);
        roi.height = tFrame.rows;
        roi.x = (tFrame.cols - roi.width) / 2;
        roi.y = 0;
    }
    else {
        roi.width = tFrame.cols;
        roi.height = (int) round(tFrame.cols / (16.0 / 9.0));
        roi.x = 0;
        roi.y = (tFrame.rows - roi.height) / 2;
    }
    cv::Mat cFrame = tFrame(roi);
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
    double maxX = 0, maxY = 0, maxA = 1;
    stableTransforms = tl::no_motion(transforms, &maxX, &maxY, &maxA);

    cv::Mat oFrame, sFrame; // original and stabilized frame
    cv::Mat transMatrix(2, 3, CV_64F); // transformation matrix that stabilizes frame

    // stabilize frames
    for (int i = 0; i < imageNames.size(); i++) {
        oFrame = cv::imread(inputPath + imageNames.at(i));
        if (i < stableTransforms.size())
            stableTransforms.at(i).matrix(transMatrix);
        sFrame = tl::transform_frame(oFrame, transMatrix, (int) round(maxX), (int) round(maxY), maxA);
        cv::imwrite(STABILIZED_TMP_FOLDER + imageNames.at(i), sFrame);
    }
}
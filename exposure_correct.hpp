#ifndef ZPO_EXP_CORRECT
#define ZPO_EXP_CORRECT

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/stitching/detail/exposure_compensate.hpp>

#define EXP_CORRECTED_TMP_FOLDER "./tmp_exp_corrected/"

namespace tl {
    void exposure_correct(std::string inputPath, std::vector<std::string> imageNames);
}

#endif //ZPO_EXP_CORRECT
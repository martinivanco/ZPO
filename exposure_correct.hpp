#ifndef ZPO_EXP_CORRECT
#define ZPO_EXP_CORRECT

#include <cstdlib>
#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#define EXP_CORRECTED_TMP_FOLDER "./tmp_exp_corrected/"

namespace tl {
    void exposure_correct(std::string inputPath, std::vector<std::string> imageNames);
}

#endif //ZPO_EXP_CORRECT
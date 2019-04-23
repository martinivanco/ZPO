#ifndef ZPO_STABILIZE
#define ZPO_STABILIZE

#include <cstdlib>
#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#define STABILIZED_TMP_FOLDER "./tmp_stabilized/"

namespace tl {
    void stabilize(std::string inputPath, std::vector<std::string> imageNames);
}

#endif //ZPO_STABILIZE
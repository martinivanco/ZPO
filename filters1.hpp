#ifndef ZPO_FILTERS1
#define ZPO_FILTERS1

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "filter.hpp"

using namespace std;
using namespace cv;

#define F1_HELP "contrast\t<value>\n"
#define F1_ARR {"denoise", "contrast", "stylize"}
#define F1_CNT 3

class ContrastFilter: public Filter {
    private:
        int alpha;
    public:
        ContrastFilter(std::vector<std::string> args);
        int filterFrame(cv::Mat &original, cv::Mat &processed);
};

#endif // ZPO_FILTERS1

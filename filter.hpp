#ifndef ZPO_FILTER
#define ZPO_FILTER

#include <opencv2/core.hpp>
#include <string>

class Filter {
    public:
        virtual int filterFrame(cv::Mat &original, cv::Mat &processed);
};

class SampleFilter: public Filter {
    private:
        int sample;
    public:
        SampleFilter(std::vector<std::string> args);
};

#endif // ZPO_FILTER
#include "filter.hpp"

int Filter::filterFrame(cv::Mat &original, cv::Mat &processed) {
    original.copyTo(processed);
    return 0;
}

SampleFilter::SampleFilter(std::vector<std::string> args) {
    this->sample = 0;
}

int SampleFilter::filterFrame(cv::Mat &original, cv::Mat &processed) {
    original.copyTo(processed);
    return 0;
}
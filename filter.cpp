#include "filter.hpp"

SampleFilter::SampleFilter(std::vector<std::string> args) {
    this->sample = 0;
}

int SampleFilter::Filter::filterFrame(cv::Mat &original, cv::Mat &processed) {
    original.copyTo(processed);
    return 0;
}
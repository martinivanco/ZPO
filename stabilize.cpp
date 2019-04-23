#include "stabilize.hpp"

void tl::stabilize(std::string inputPath, std::vector<std::string> imageNames) {
    std::string command("mkdir -p ");
    command += STABILIZED_TMP_FOLDER;
    system(command.c_str());
    
    for(int i = 0; i < imageNames.size(); i++) {
        cv::Mat image = cv::imread(inputPath + imageNames.at(i));
        
        // TODO PROCESSING

        cv::Mat output = image;
        cv::imwrite(STABILIZED_TMP_FOLDER + imageNames.at(i), output);
    }
}
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include "stabilize.hpp"
#include "exposure_correct.hpp"

#define HELP_MSG "Usage:\ntimelapse <input folder path> <framerate> <output file path>"

using namespace std;
using namespace cv;

int parseArgs(int nArgs, char **args, string &in, string &out, vector<string> &images, int *fps, string &err) {
    for (int i = 1; i < nArgs; i++) {
        if (strcmp(args[i], "--help") == 0)
            return 2;
    }

    if (nArgs != 4) {
        err = "This program requires 3 parameters.";
        return 1;
    }

    in = args[1];
    DIR* dir = opendir(args[1]);
    struct dirent *ent;
    if (dir) {
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_name[0] == '.')
                continue;
            string filename(ent->d_name);
            size_t pos = 0;
            string part = "";
            while ((pos = filename.find(string("."))) != string::npos) {
                part = filename.substr(0, pos);
                filename.erase(0, pos + 1);
            }
            if ((part.empty()) || (filename.compare("jpg") != 0))
                continue;
            images.push_back(ent->d_name);
        }
        sort(images.begin(), images.end());
        closedir(dir);
    } else if (ENOENT == errno) {
        err = "Input path is not a folder.";
        return 1;
    } else {
        err = "Input path is invalid.";
        return 1;
    }

    try {
        *fps = stoi(string(args[2]));
        if (*fps < 1)
            throw invalid_argument("must be positive");
    }
    catch (invalid_argument e) {
        err = "Framerate must be a positive integer.";
        return 1;
    }

    out = args[3];
    return 0;
}

int main(int argc, char **argv) {
    // parse arguments
    string inputPath, outputPath, errorMsg;
    vector<string> imageNames;
    int framerate;

    int result = parseArgs(argc, argv, inputPath, outputPath, imageNames, &framerate, errorMsg);
    if (result == 1) cerr << "ERROR: " << errorMsg << "\nUse --help to display usage and available filters." << endl;
    if (result == 2) cout << HELP_MSG << endl;
    if (result != 0) return result;

    // stabilize frames
    tl::stabilize(inputPath + "/", imageNames);

    // exposure correct frames
    tl::exposure_correct(STABILIZED_TMP_FOLDER, imageNames);

    // get rid of temporary stabilized images
    string command("rm -rf ");
    command += STABILIZED_TMP_FOLDER;
    system(command.c_str());

    // output video
    Mat testsize = imread(EXP_CORRECTED_TMP_FOLDER + imageNames.at(0));
    VideoWriter output(outputPath.c_str(), VideoWriter::fourcc('a','v','c','1'), framerate, testsize.size());
    for(int i = 0; i < imageNames.size(); i++) {
        Mat frame = imread(EXP_CORRECTED_TMP_FOLDER + imageNames.at(i));
        output.write(frame);
    }

    // get rid of temporary exposure corrected images
    command = "rm -rf ";
    command += EXP_CORRECTED_TMP_FOLDER;
    system(command.c_str());

    output.release();
    return 0;
}
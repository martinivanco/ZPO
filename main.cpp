#include <string.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include "filter.hpp"
#include "filters1.hpp"
#include "filters2.hpp"

#define HELP_MSG "Usage:\nvidfilter <input file path> <filter name> [<filter arguments>] <output file path>\nAvailable filters:\n"

using namespace std;
using namespace cv;

int parseArgs(int nArgs, char **args, string &in, int *filter, vector<string> &filterArgs, string &out, string &err) {
    for (int i = 1; i < nArgs; i++) {
        if (strcmp(args[i], "--help") == 0)
            return 2;
    }

    if (nArgs < 4) {
        err = "Not enough parameters.";
        return 1;
    }
    in = args[1];
    out = args[nArgs - 1];
    string filterName = args[2];

    string filters1[F1_CNT] = F1_ARR;
    for (int i = 0; i < F1_CNT; i++) {
        if (filterName.compare(filters1[i]) == 0) {
            *filter = 10 + i;
            for (int j = 3; j < nArgs - 1; j++)
                filterArgs.push_back(string(args[j]));
            return 0;
        }
    }

    string filters2[F2_CNT] = F2_ARR;
    for (int i = 0; i < F2_CNT; i++) {
        if (filterName.compare(filters2[i]) == 0) {
            *filter = 20 + i;
            for (int j = 3; j < nArgs - 1; j++)
                filterArgs.push_back(string(args[j]));
            return 0;
        }
    }

    err = "Unknown filter.";
    return 1;
}

int initFilter(Filter **filter, int filterCode, vector<string> filterArgs) {
    *filter = new SampleFilter(filterArgs); // TODO after filters are implemented this is unnecessary
    switch(filterCode) {
        case 10:
            // TODO
            break;
        case 11:
            cout << "Using filter 'contrast'." << endl;
            *filter = new ContrastFilter(filterArgs);
            break;
        case 12:
            // TODO
            break;
        case 20:
            // TODO
            break;
        case 21:
            // TODO
            break;
        case 22:
            // TODO
            break;
        default:
            // return 1; // error
            break;
    }
    return 0; // TODO after filters are implemented this will be error
}

int main(int argc, char **argv) {
    // parse arguments
    string inputPath, outputPath, errorMsg;
    int filterCode;
    vector<string> filterArgs;
    int result = parseArgs(argc, argv, inputPath, &filterCode, filterArgs, outputPath, errorMsg);
    if (result == 1) cerr << "ERROR: " << errorMsg << "\nUse --help to display usage and available filters." << endl;
    if (result == 2) cout << HELP_MSG << F1_HELP << F2_HELP << endl;
    if (result != 0) return result;

    Mat original, processed;
    // initialize video IO
    VideoCapture input(inputPath.c_str());
    if (!input.isOpened()) {
        cerr << "ERROR: Could not open input file." << endl;
        return 1;
    }
    input.read(original);
    if (original.empty()) {
        cerr << "ERROR: File does not contain valid frames." << endl;
        return 1;
    }
    VideoWriter output(outputPath.c_str(), CV_FOURCC('a','v','c','1'), input.get(CAP_PROP_FPS), original.size());

    // initialize filter
    Filter *filter;
    result = initFilter(&filter, filterCode, filterArgs);
    if (result != 0) return result;

    // main loop
    int print_frame = input.get(CAP_PROP_FRAME_COUNT) / 20;
    int counter = 1;
    cout << "Render progress: [                    ]\r" << flush;
    while (!original.empty()) {
        
        // print progress
        if (counter % print_frame == 0) {
            cout << "Render progress: [";
            int i = 0;
            for (; i < counter / print_frame; i++)
                cout << "=";
            for (int j = 0; j < 20 - i; j++)
                cout << " ";
            cout << "]\r" << flush;
        }

        // render
        filter->filterFrame(original, processed);
        output.write(processed);
        input.read(original);
        counter++;
    }
    cout << "Render finished." << endl;

    // clean up
    input.release();
    output.release();
    return 0;
}
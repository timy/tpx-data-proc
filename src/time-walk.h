#pragma once

#include <vector>
#include "pixel.h"

using namespace std;

class TimeWalkCorrector {
public:
    TimeWalkCorrector(vector<Pixel>* pixels);
    TimeWalkCorrector(const char* filename);
    virtual ~TimeWalkCorrector();
    void process();
    void dump_to_file(const char* filename);

private:
    vector<Pixel>* pixels;
    bool require_delete;
};
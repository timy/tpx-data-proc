#include <fstream>
#include "time-walk.h"

#include <boost/math/special_functions/lambert_w.hpp>
using boost::math::lambert_w0;

TimeWalkCorrector::TimeWalkCorrector(vector<Pixel>* pixels)
    : pixels(pixels), require_delete(false) {
}

TimeWalkCorrector::TimeWalkCorrector(const char* filename) 
    : require_delete(true) {

    ifstream file(filename);
    if (!file) {
        cout << "TimeWalkCorrector: cannot read file \"" << filename << "\"" << endl;
        exit(-1);
    }
    string line;
    Pixel pixel;
    pixels = new vector<Pixel>;
    getline(file, line); // reference time stamp from the TDC
    while (!file.eof()) {
        file >> pixel.toa;
        file >> pixel.tot;
        file >> pixel.x;
        file >> pixel.y;
        pixels->push_back(pixel);
    }
    file.close();
}

TimeWalkCorrector::~TimeWalkCorrector() {
    if (require_delete) {
        delete pixels;
    }    
}

void TimeWalkCorrector::process() {
    const double k = 1./ 5e-6;
    const double d1 = 3e-7;
    for (auto& pixel : *pixels) {
        double TOT = pixel.tot * 25e-9;
        double delta = lambert_w0(k * d1 * exp(k * (d1 - TOT))) / k;
        pixel.toa -= delta;
    }
}

void TimeWalkCorrector::dump_to_file(const char* filename) {
    ofstream file(filename);
    for (auto pixel : *pixels)
        file << setprecision(15) << pixel;
    file.close();    
}
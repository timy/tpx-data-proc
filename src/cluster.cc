#include "cluster.h"
#include <fstream>
#include <string>

using namespace std;

Clusterizer::Clusterizer(vector<RawPixel>* pixels) 
    : n_clusters(0), pixels_in(pixels), require_delete(false) {
}

Clusterizer::Clusterizer(const char* filename) 
    : n_clusters(0), require_delete(true) {

    ifstream file(filename);
    string line;
    RawPixel pixel;
    pixels_in = new vector<RawPixel>;
    getline(file, line); // reference time stamp from the TDC
    while (!file.eof()) {
        file >> pixel.toa;
        file >> pixel.tot;
        file >> pixel.x;
        file >> pixel.y;
        pixels_in->push_back(pixel);
    }
    file.close();
}

Clusterizer::~Clusterizer() {
    if (require_delete) {
        delete pixels_in;
    }
}

void Clusterizer::process() {
    for (auto& pixel : *pixels_in) {
        if (pixel.clst_id == -1) {
            findNext(pixel, n_clusters);
            n_clusters ++;
        }
    }

    clusters.resize(n_clusters);
    for (auto& pixel : *pixels_in) {
        clusters[pixel.clst_id].addPixel(pixel);
    }

    // pixels_out.resize(n_clusters);
    pixels_out.clear();
    for (auto& cluster : clusters) {
        cluster.geom_center();
        cluster.centroiding();
        cluster.characterize();
        Pixel p;
        p.x = cluster.xc;
        p.y = cluster.yc;
        p.toa = cluster.pixels[cluster.idx_max_tot].toa;
        p.tot = cluster.pixels[cluster.idx_max_tot].tot;
        pixels_out.push_back(p);
    }
}

void Clusterizer::findNext(RawPixel& pixel, int clsId) {
    pixel.clst_id = clsId;
    for (auto& pixelNext : *pixels_in) {
        if (pixelNext.clst_id < 0) {
            if (abs(pixel.x - pixelNext.x) <= 1 && 
                abs(pixel.y - pixelNext.y) <= 1 && 
                fabs(pixel.toa - pixelNext.toa) <= 1e-6) {
                findNext(pixelNext, clsId);
            }
        }
    }
}

void Clusterizer::check() {
    for (size_t i = 0; i < 10; i ++) {
        cout << (*pixels_in)[i].toa << " " << (*pixels_in)[i].tot 
            << " (" << (*pixels_in)[i].x << "," << (*pixels_in)[i].y << ") "
            << (*pixels_in)[i].clst_id << " " << (*pixels_in)[i].chip_id << endl;
    }
}

void Clusterizer::dump_to_file(const char* filename) {
    ofstream file(filename);
    for (auto pixel : pixels_out)
        file << pixel;
    file.close();
}

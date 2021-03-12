#pragma once

#include <vector>
#include <cmath>
#include <iostream>

using namespace std;

class Pixel {
public:	
	Pixel() : x(0), y(0), toa(0.), tot(0.) {}
	virtual ~Pixel() {}
	friend ostream& operator << (ostream& os, const Pixel& pixel);

	int x;
	int y;
	double toa;
	double tot;
};
 
class RawPixel : public Pixel {
public:
	RawPixel() : clst_id(-1), chip_id(-1) {};
	virtual ~RawPixel() {};

	int clst_id; // cluster ID
	int chip_id;
};

class Cluster {
public:

	Cluster() : xc(0), yc(0), gcx(0), gcy(0) {
		pixels.clear();
	}

	virtual ~Cluster() { 
		pixels.clear();
	}
	
	void addPixel(RawPixel p);

	void characterize();

	// calculate the centroiding center of the cluster
	void centroiding();

	// calculate the geometry center of the cluster
	void geom_center();
	
	double xc, yc;
	double gcx, gcy;
	int n_pixels;
	int idx_max_tot;
	int idx_min_toa;
	double sum_tot;
	vector <Pixel> pixels;
};

class Clusterizer {
public:
    Clusterizer(vector<RawPixel>* pixels);
	Clusterizer(const char* filename);
    virtual ~Clusterizer();

    void process();
	void check();
	void dump_to_file(const char* filename);

private:
    int n_clusters;
	vector<RawPixel>* pixels_in;
	vector<Pixel> pixels_out;
	vector<Cluster> clusters;
	bool require_delete;
	void findNext(RawPixel& pixel, int clsId);
	void centroiding();
};
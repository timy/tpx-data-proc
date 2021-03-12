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

ostream& operator << (ostream& os, const Pixel& pixel) {
	os << pixel.toa << " " << pixel.tot << " " << pixel.x << " " << pixel.y << endl;
	return os;
}
 
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
	
	void addPixel(RawPixel p) {
		Pixel pixel;
		pixel.x = p.x;
		pixel.y = p.y;
		pixel.toa = p.toa;
		pixel.tot = p.tot;
		pixels.push_back(p);
	};

	// pick up the pixel with the max ToT
	void characterize() {
		n_pixels = pixels.size();   // number of pixels within the cluster
		idx_max_tot = 0;
		double max_tot = pixels[0].tot;
		idx_min_toa = 0;
		double min_toa = pixels[0].toa;

		int count = 0;
		sum_tot = 0.;
		for (auto pixel : pixels) {
			if (pixel.tot > max_tot) {
				max_tot = pixel.tot;
				idx_max_tot = count; // index of the smallest ToT
			}
			if (pixel.toa < min_toa) {
				min_toa = pixel.toa;
				idx_min_toa = count; // index of the largest ToA
			}
			sum_tot += pixel.tot;    // ToT sum over the whole cluster
			count ++;
		}
	};

	// calculate the centroiding center of the cluster
	void centroiding() {
		double delta_tot_sum = 0;
		xc = 0;
		yc = 0;
		for (auto pixel : pixels) {
			xc += pixel.tot * pixel.x;
			yc += pixel.tot * pixel.y;
			delta_tot_sum += pixel.tot;
		}
		xc /= delta_tot_sum;
		yc /= delta_tot_sum;		
	}

	// calculate the geometry center of the cluster
	void geom_center() {
		gcx = 0;
		gcy = 0;
		for (auto pixel : pixels) {
			gcx += pixel.x;
			gcy += pixel.y;
		}
		gcx /= pixels.size();
		gcy /= pixels.size();
	}
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
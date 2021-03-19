#pragma once

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
#pragma once

#include <fstream>

class TPX3Parser {
public:
	TPX3Parser(const char* filename_input, const char* filename_output);
	virtual ~TPX3Parser();
	void process();

private:
	char HeaderBuffer[8];
	unsigned long long curr_packet; // each data frame of unsigned long long = 8 bytes (64 bits)
	char chipnr;
	unsigned short spidrTime;
	unsigned long count_TDCs;
	// unsigned long count_pixels;
	unsigned long long file_size;
	unsigned long long n_packets;
	std::ifstream file_tpx;
	std::ofstream file_out;
	std::ofstream file_dbg;

	inline void proc_pixel_data();
	inline void proc_TDC_trigger();
	inline void proc_global_time();
};
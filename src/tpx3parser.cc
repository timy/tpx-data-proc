#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <bitset>
#include "tpx3parser.h"

using namespace std;

template<typename VType>
	string to_binary (VType value) {
	bitset<sizeof(VType)*8> bs (value);
	return bs.to_string ();
}

unsigned long long get_file_size(ifstream& file) {
	file.seekg(0, file.end);                       // relocate at the end of the file
	unsigned long long file_size = file.tellg();   // return the current position, i.e., the file size
	file.seekg(0, file.beg);                       // go back to the beginning of the file
	cout << "file size: " << file_size << " Bytes (" << 1. * file_size / (1024 * 1024) << " MB)" << endl;
	return file_size;
}

TPX3Parser::TPX3Parser(const char* filename_input, const char* filename_output)
	: count_TDCs(0) {
	file_tpx.open(filename_input, ios::binary);
	if (!file_tpx) {
		cout << "Raw data file \"" << filename_input << "\" is not accessible." << endl;
		exit(-1);
	}
	file_out.open(filename_output);
	if (!file_out) { 
		cout << "Output file \"" << filename_output << "\" is not accessible." << endl;
		exit(-1); 
	}
	file_size = get_file_size(file_tpx);
	n_packets = file_size / 8;
}
TPX3Parser::~TPX3Parser() {
	if (!file_tpx) file_tpx.close();
	if (!file_out) file_out.close();
}

void TPX3Parser::process() {

	while (!file_tpx.eof()) {

		file_tpx.read((char*)&curr_packet, sizeof(unsigned long long));
		for (int i = 0; i < 8; i ++) {
			HeaderBuffer[i] = (char)(curr_packet >> 8*i);
			// file_test << to_binary<short> (HeaderBuffer[i]) << " (" << HeaderBuffer[i] << ")" << "\t";
		}			
		
		if (HeaderBuffer[0] == 'T' && HeaderBuffer[1] == 'P' && HeaderBuffer[2] == 'X') {
			// size of data chunks after the header
			int size = ((0xff & HeaderBuffer[7]) << 8) | (0xff & HeaderBuffer[6]); 
			chipnr = HeaderBuffer[4]; // chip number
			// int mode = HeaderBuffer[5]; // data driven mode

			// read the data chunk
			for (int j = 0; j < size / 8; j ++) {
				file_tpx.read((char*)&curr_packet, sizeof(unsigned long long));
				// file_test << to_binary <unsigned long long> (curr_packet) << endl;
				int packet_type = curr_packet >> 60; // the leftest 4 bits
				switch (packet_type) {
				case 0xb:
					proc_pixel_data();
					break;
				case 0x6:
					proc_TDC_trigger();
					count_TDCs ++;
					break;
				case 0x4:
					proc_global_time();
					break;
				default:
					break;
				}
			}
		}
	}

}

inline void TPX3Parser::proc_pixel_data() {

	spidrTime = (unsigned short)(curr_packet & 0xffff);
	long dcol = (curr_packet & 0x0FE0000000000000L) >> 52;                                                                  
	long spix = (curr_packet & 0x001F800000000000L) >> 45;                                                                    
	long pix  = (curr_packet & 0x0000700000000000L) >> 44;
	int x = (int)(dcol + pix / 4);
	int y = (int)(spix + (pix & 0x3));
	unsigned short TOA = (unsigned short)((curr_packet >> (16 + 14)) & 0x3fff);   
	unsigned short TOT = (unsigned short)((curr_packet >> (16 + 4)) & 0x3ff);	
	char FTOA = (unsigned char)((curr_packet >> 16) & 0xf);
	int CTOA = (TOA << 4) | (~FTOA & 0xf); // CTOA = TOA * 16 + ~FTOA
	double spidrTimens = spidrTime * 25.0 * 16384.0; // 25*2^14
	// double TOAns = TOA * 25.0;
	double TOTns = TOT * 25.0;	
	double global_timestamp = spidrTimens + CTOA * (25.0 / 16);
	/*					
	switch ((int)chipnr) { // used only for quad chips
	case 0:
		x += 260;
		y = y;
		break;
	case 1:
		x = 255 - x + 260;
		y = 255 - y + 260;
		break;
	case 2:
		x = 255 - x;
		y = 255 - y + 260;
		break;
	case 3:
		break;
	default:
		break;					
	}
	*/
	file_out << setprecision(15) << global_timestamp / 1e9 << "\t\t" 
			<< TOTns << "\t" << x << "\t" << y << endl;
}

inline void TPX3Parser::proc_TDC_trigger() {
	
	int hdr = (int)(curr_packet >> 56); // the leftest 8 bits
	/*
	switch (hdr) {
		case 0x6f: cout << "tdc1 rising edge" << endl; break;
		case 0x6a: cout << "tdc1 falling edge" << endl; break;
		case 0x6e: cout << "tdc2 rising edge" << endl; break;
		case 0x6b: cout << "tdc2 falling edge" << endl;	break;
	}
	*/
	double coarsetime = (curr_packet >> 12) & 0xFFFFFFFF;       
	unsigned long tmpfine = (curr_packet >> 5) & 0xF; 
	tmpfine = ((tmpfine - 1) << 9) / 12;
	unsigned long trigtime_fine = (curr_packet & 0x0000000000000E00) | (tmpfine & 0x00000000000001FF);
	double time_unit = 25. / 4096;
	// int trigger_counter = curr_packet >> 44 & 0xFFF; 
	double TDC_timestamp = coarsetime * 25E-9 + trigtime_fine * time_unit*1E-9;
	if (hdr == 0x6a) // record at the falling edge of TDC1
		file_out << setprecision(15) << TDC_timestamp << endl;
}

inline void TPX3Parser::proc_global_time() {
	static unsigned long Timer_LSB32, Timer_MSB16;
	if (((curr_packet >> 56) & 0xF) == 0x4) {
		Timer_LSB32 = (curr_packet >> 16) & 0xFFFFFFFF;
	} else if (((curr_packet >> 56) & 0xF) == 0x5) {
		Timer_MSB16 = (curr_packet >> 16) & 0xFFFF;
		unsigned long long int timemaster;
		timemaster = Timer_MSB16;
		timemaster = (timemaster << 32) & 0xFFFF00000000;
		timemaster = timemaster | Timer_LSB32;
		int diff = (spidrTime >> 14) - ((Timer_LSB32 >> 28) & 0x3);

		if ((spidrTime >> 14) == (unsigned short)((Timer_LSB32 >> 28) & 0x3)) { 						
		} else {                               
			Timer_MSB16 = Timer_MSB16 - diff;
		}  
		// uncomment below to save the global timestamps into the text file;
		file_out << " Global time: " << setprecision(15) << timemaster * 25e-9 << endl;
	}	
}
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include "splitter.h"

namespace fs = std::filesystem;

Splitter::Splitter(const char* filename_input, std::filesystem::path dir_output) 
    : count_valves(0), count_lasers(0), count_events(0), tdc_count(0), 
    count_lines(0), count_lasers_all(0), count_events_all(0),
    res_dir(dir_output), state(S_ENTRY) {

	file_src_data.open(filename_input);
	if (!file_src_data) {
		std::cout << "Input data file \"" << filename_input << "\" is not accessible." << std::endl;
		exit(-1);
	}
}

Splitter::~Splitter() {
    if (!file_src_data) file_src_data.close();
}

inline void Splitter::write_float_value(std::ofstream& file, double val) {
    file << std::setw(15) << std::fixed << std::setprecision(9) << val;
}

inline void Splitter::end_of_event_chunk() { 
    count_events_all += count_events;
    arr_count_events.push_back(count_events);
    count_events = 0; 
    tdc_count ++;
    tdc_time = data[0];
}

inline void Splitter::consecutive_event() { 
    count_events ++; 
    write_event_data_line();
}

inline void Splitter::dump_valve_block_info() {
    // file_res_info << std::setw(10) << count_valves - 1;
    file_res_info << std::setw(6) << count_lasers;
    for (int v : arr_count_events)
        file_res_info << std::setw(6) << v;
    file_res_info << std::endl;
    arr_count_events.clear();
}

Splitter::Input Splitter::get_input_type(int nCol) {
    switch (nCol) {
    case 1:
        return INPUT_TDC;
    case 4:
        return INPUT_EVE;
    default:
        std::cerr << "Unknown INPUT type for n_col = " << nCol << std::endl;
        exit(-1);
    }
}

void Splitter::next(int n_col) {
    Input input = get_input_type(n_col);
    state_old = state;
    (this->*operate[state][input])(); // operation for such a specific transition (state, input)
    state = fsm_map[state][input];    // map to the next state: (old_state, input) -> new_state
    count_lines ++;
    // show_FSM();
}

void Splitter::show_FSM() {
    file_res_info << state_name[state_old] << " -> " << state_name[state] << std::endl;
}

void Splitter::process() {

    std::string line;
    std::istringstream ss;

    std::string filename_tmp = res_dir.string() + "/info.tmp";

    file_res_info.open(filename_tmp);

    while (std::getline(file_src_data, line)) {
        ss.clear();
        ss.str(line);
        int n_col = 0;
        while (ss >> data[n_col])
            n_col ++;
        next(n_col);
    }
    // the remaining information not yet processed during the FWM flow
    count_events_all += count_events;
    arr_count_events.push_back(count_events);
    count_lasers_all += count_lasers;
    dump_valve_block_info();

    file_res_info.close();

    // prepend data_info to new info_file and transfer the detailed information from the temp_info_file
    file_res_info.open(res_dir.string() + "/info.dat");
    file_res_info << "n_valves " << std::setw(20) << count_valves << std::endl;
    file_res_info << "n_lasers_all " << std::setw(16) << count_lasers_all << std::endl;
    file_res_info << "n_events_all " << std::setw(16) << count_events_all << std::endl;
    file_res_info << "n_lines " << std::setw(21) << count_lines << std::endl;
    file_res_info << std::endl;
    std::ifstream file_tmp(filename_tmp);
    while (std::getline(file_tmp, line)) {
        file_res_info << line << std::endl;
    }
    file_tmp.close();
    file_res_info.close();
    fs::remove(fs::path(filename_tmp));
}

void Splitter::write_event_data_line() {
    write_float_value(file_res_data, data[0]);
    file_res_data << std::setw(10) << int(data[1]); 
    file_res_data << std::setw(10) << int(data[2]);
    file_res_data << std::setw(10) << int(data[3]);
    file_res_data << std::endl;
}

void Splitter::open_event_data_file(int i_valve, int i_laser) {
    if (file_res_data.is_open())
        file_res_data.close();

    std::stringstream ss;
    ss << res_dir.string() << "/data_";
    ss << std::setfill('0') << std::setw(6) << i_valve;
    ss << "_";
    ss << std::setfill('0') << std::setw(6) << i_laser;
    ss << ".dat";
    std::string filename;
    ss >> filename;
    file_res_data.open(filename);
}

void Splitter::start_new_event_data_file(int i_valve, int i_laser) {
    open_event_data_file(i_valve, i_laser);
    write_float_value(file_res_data, tdc_time);
    file_res_data << std::endl;
    write_event_data_line();
}

void Splitter::do_ENTRY_TDC() { tdc_count ++; tdc_time = data[0]; }
void Splitter::do_ENTRY_EVE() { std::cout << "Unknown for S_ENTRY with INPUT_EVE." << std::endl; exit(-1); }
void Splitter::do_INITTDC_TDC() { tdc_count ++; tdc_time = data[0]; }
void Splitter::do_INITTDC_EVE() { 
    // start of a valve pulse, dump time stamp into the info_file
    write_float_value(file_res_info, tdc_time);

    count_valves ++; count_lasers ++; count_events ++; tdc_count = 0;
    start_new_event_data_file(count_valves-1, count_lasers-1);
}
void Splitter::do_TDC0_TDC() { 
    tdc_count ++; tdc_time = data[0];
    // end of a valve pulse, dump number of events in each pulse within the valve into the info_file
    dump_valve_block_info();
    count_lasers_all += count_lasers; count_lasers = 0;
}
void Splitter::do_TDC0_EVE() { 
    count_lasers ++; count_events ++; tdc_count = 0;
    start_new_event_data_file(count_valves-1, count_lasers-1);
}
void Splitter::do_TDC1_TDC() { tdc_count ++; tdc_time = data[0]; }
void Splitter::do_TDC1_EVE() {
    // start of a valve pulse, dump time stamp into the info_file
    write_float_value(file_res_info, tdc_time);

    count_valves ++; count_lasers ++; count_events ++; tdc_count = 0; 
    start_new_event_data_file(count_valves-1, count_lasers-1);
}
void Splitter::do_EVE0_TDC() { end_of_event_chunk(); }
void Splitter::do_EVE0_EVE() { consecutive_event(); }
void Splitter::do_EVE1_TDC() { end_of_event_chunk(); }
void Splitter::do_EVE1_EVE() { consecutive_event(); }
void Splitter::do_EVE2_TDC() { end_of_event_chunk(); }
void Splitter::do_EVE2_EVE() { consecutive_event(); }
void Splitter::do_EVE3_TDC() { end_of_event_chunk(); }
void Splitter::do_EVE3_EVE() { consecutive_event(); }
void Splitter::do_UNKN_TDC() { std::cerr << "Unknown for S_UNKN with INPUT_TDC." << std::endl; exit(-1); }
void Splitter::do_UNKN_EVE() { std::cerr << "Unknown for S_UNKN with INPUT_EVE." << std::endl; exit(-1); }
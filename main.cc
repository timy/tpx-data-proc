#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

void create_res_directory(const fs::path& res_dir) {
    if (!fs::exists(res_dir)) {
        fs::create_directory(res_dir);
    } else {
        if (!fs::is_directory(res_dir)) {
            fs::create_directory(res_dir);
        } else {
            std::cout << "Found an existing directory " << res_dir << std::endl;
            std::cout << "Remove it first and retry." << std::endl;
            exit(-1);
        }
    }
}

struct DP {

    enum State {
        S_ENTRY,
        S_INITTDC, // the initial TDC
        S_TDC0,    // the first TDC after a event
        S_TDC1,    // the TDC after a TDC
        S_EVE0,    // globally the first event
        S_EVE1,    // the first event within a valve cycle and a laser pulse
        S_EVE2,    // the first event within a laser pulse
        S_EVE3,    // the event after an event
        S_UNKN,    // unknown
        N_TYPES_OF_STATE
    };
    enum Input {
        INPUT_TDC, // 1 column
        INPUT_EVE, // 4 columns
        N_TYPES_OF_INPUT
    };

    const char* state_name[N_TYPES_OF_STATE] = {
        "S_ENTRY",
        "S_INITTDC",
        "S_TDC0",
        "S_TDC1",
        "S_EVE0",
        "S_EVE1",
        "S_EVE2",
        "S_EVE3",
        "S_UNKN"
    };

    const State fsm_map[N_TYPES_OF_STATE][N_TYPES_OF_INPUT] = {
        {S_INITTDC, S_UNKN}, 
        {S_INITTDC, S_EVE0}, 
        {S_TDC1   , S_EVE2},
        {S_TDC1   , S_EVE1},
        {S_TDC0   , S_EVE3},
        {S_TDC0   , S_EVE3},
        {S_TDC0   , S_EVE3},
        {S_TDC0   , S_EVE3},
        {S_UNKN   , S_UNKN}
    };

    DP() : count_valves(0), count_lasers(0), count_events(0), tdc_count(0), 
    count_lines(0), count_lasers_all(0), count_events_all(0),
    res_dir(fs::current_path() / "res/"), state(S_ENTRY) {
        create_res_directory(res_dir);
    }
    virtual ~DP() {
    }

    inline void write_float_value(std::ofstream& file, double val) {
        file << std::setw(15) << std::fixed << std::setprecision(9) << val;
    }

    inline void end_of_event_chunk() { 
        count_events_all += count_events;
        arr_count_events.push_back(count_events);
        count_events = 0; 
        tdc_count ++;
        tdc_time = data[0];
    }
    inline void consecutive_event() { 
        count_events ++; 
        write_event_data_line();
    }

    inline void dump_valve_block_info() {
        // file_res_info << std::setw(10) << count_valves - 1;
        file_res_info << std::setw(6) << count_lasers;
        for (int v : arr_count_events)
            file_res_info << std::setw(6) << v;
        file_res_info << std::endl;
        arr_count_events.clear();
    }

    void do_ENTRY_TDC() { tdc_count ++; tdc_time = data[0]; }
    void do_ENTRY_EVE() { std::cout << "Unknown for S_ENTRY with INPUT_EVE." << std::endl; exit(-1); }
    void do_INITTDC_TDC() { tdc_count ++; tdc_time = data[0]; }
    void do_INITTDC_EVE() { 
        // start of a valve pulse, dump time stamp into the info_file
        write_float_value(file_res_info, tdc_time);

        count_valves ++; count_lasers ++; count_events ++; tdc_count = 0;
        start_new_event_data_file(count_valves-1, count_lasers-1);
    }
    void do_TDC0_TDC() { 
        tdc_count ++; tdc_time = data[0];
        // end of a valve pulse, dump number of events in each pulse within the valve into the info_file
        dump_valve_block_info();
        count_lasers_all += count_lasers; count_lasers = 0;
    }
    void do_TDC0_EVE() { 
        count_lasers ++; count_events ++; tdc_count = 0;
        start_new_event_data_file(count_valves-1, count_lasers-1);
    }
    void do_TDC1_TDC() { tdc_count ++; tdc_time = data[0]; }
    void do_TDC1_EVE() {
        // start of a valve pulse, dump time stamp into the info_file
        write_float_value(file_res_info, tdc_time);

        count_valves ++; count_lasers ++; count_events ++; tdc_count = 0; 
        start_new_event_data_file(count_valves-1, count_lasers-1);
    }
    void do_EVE0_TDC() { end_of_event_chunk(); }
    void do_EVE0_EVE() { consecutive_event(); }
    void do_EVE1_TDC() { end_of_event_chunk(); }
    void do_EVE1_EVE() { consecutive_event(); }
    void do_EVE2_TDC() { end_of_event_chunk(); }
    void do_EVE2_EVE() { consecutive_event(); }
    void do_EVE3_TDC() { end_of_event_chunk(); }
    void do_EVE3_EVE() { consecutive_event(); }
    void do_UNKN_TDC() { std::cerr << "Unknown for S_UNKN with INPUT_TDC." << std::endl; exit(-1); }
    void do_UNKN_EVE() { std::cerr << "Unknown for S_UNKN with INPUT_EVE." << std::endl; exit(-1); }


    void (DP::*operate[N_TYPES_OF_STATE][N_TYPES_OF_INPUT])() = {
        {&DP::do_ENTRY_TDC,   &DP::do_ENTRY_EVE},
        {&DP::do_INITTDC_TDC, &DP::do_INITTDC_EVE},
        {&DP::do_TDC0_TDC,    &DP::do_TDC0_EVE},
        {&DP::do_TDC1_TDC,    &DP::do_TDC1_EVE},
        {&DP::do_EVE0_TDC,    &DP::do_EVE0_EVE},
        {&DP::do_EVE1_TDC,    &DP::do_EVE1_EVE},
        {&DP::do_EVE2_TDC,    &DP::do_EVE2_EVE},
        {&DP::do_EVE3_TDC,    &DP::do_EVE3_EVE}, 
        {&DP::do_UNKN_TDC,    &DP::do_UNKN_EVE}
    };

    Input get_input_type(int nCol) {
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

    void next(int n_col) {
        Input input = get_input_type(n_col);
        state_old = state;
        (this->*operate[state][input])(); // operation for such a specific transition (state, input)
        state = fsm_map[state][input];    // map to the next state: (old_state, input) -> new_state
        count_lines ++;
        // show_FSM();
    }

    void show_FSM() {
        file_res_info << state_name[state_old] << " -> " << state_name[state] << std::endl;
    }

    void process(std::ifstream& file_src_data) {

        std::string line;
        std::istringstream ss;

        std::string filename_tmp = res_dir.string() + "info.tmp";

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
        file_res_info.open(res_dir.string() + "info.dat");
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

    void write_event_data_line() {
        write_float_value(file_res_data, data[0]);
        file_res_data << std::setw(10) << int(data[1]); 
        file_res_data << std::setw(10) << int(data[2]);
        file_res_data << std::setw(10) << int(data[3]);
        file_res_data << std::endl;
    }

    void open_event_data_file(int i_valve, int i_laser) {
        if (!(i_valve == 1 && i_laser == 1))
            file_res_data.close();

        std::stringstream ss;
        ss << res_dir.string() << "data_";
        ss << std::setfill('0') << std::setw(6) << i_valve;
        ss << "_";
        ss << std::setfill('0') << std::setw(6) << i_laser;
        ss << ".dat";
        std::string filename;
        ss >> filename;
        file_res_data.open(filename);
    }

    void start_new_event_data_file(int i_valve, int i_laser) {
        open_event_data_file(i_valve, i_laser);
        write_float_value(file_res_data, tdc_time);
        file_res_data << std::endl;
        write_event_data_line();
    }

    long count_valves;
    long count_lasers;
    long count_events;
    long tdc_count;

    long count_lines;
    long count_lasers_all;
    long count_events_all;

    std::vector<long> arr_count_events;

    fs::path res_dir;

    // output result data
    std::ofstream file_res_info;
    std::ofstream file_res_data;

    State state, state_old;
    double data[4];
    double tdc_time;
};

int main(int argc, char* argv[]) {

    std::ifstream file_src_data("../converted.dat"); // intput source data
    DP dp;
    dp.process(file_src_data);
    file_src_data.close();

    return 0;
}
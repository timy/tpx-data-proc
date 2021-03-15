#pragma once

#include <filesystem>
#include <vector>

class Splitter {

public:
    Splitter(const char* filename_input, std::filesystem::path dir_output);
    virtual ~Splitter();
    void process();

private:
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

    inline void write_float_value(std::ofstream& file, double val);
    inline void end_of_event_chunk();
    inline void consecutive_event();
    inline void dump_valve_block_info();

    void do_ENTRY_TDC();
    void do_ENTRY_EVE();
    void do_INITTDC_TDC();
    void do_INITTDC_EVE();
    void do_TDC0_TDC();
    void do_TDC0_EVE();
    void do_TDC1_TDC();
    void do_TDC1_EVE();
    void do_EVE0_TDC();
    void do_EVE0_EVE();
    void do_EVE1_TDC();
    void do_EVE1_EVE();
    void do_EVE2_TDC();
    void do_EVE2_EVE();
    void do_EVE3_TDC();
    void do_EVE3_EVE();
    void do_UNKN_TDC();
    void do_UNKN_EVE();

    void (Splitter::*operate[N_TYPES_OF_STATE][N_TYPES_OF_INPUT])() = {
        {&Splitter::do_ENTRY_TDC,   &Splitter::do_ENTRY_EVE},
        {&Splitter::do_INITTDC_TDC, &Splitter::do_INITTDC_EVE},
        {&Splitter::do_TDC0_TDC,    &Splitter::do_TDC0_EVE},
        {&Splitter::do_TDC1_TDC,    &Splitter::do_TDC1_EVE},
        {&Splitter::do_EVE0_TDC,    &Splitter::do_EVE0_EVE},
        {&Splitter::do_EVE1_TDC,    &Splitter::do_EVE1_EVE},
        {&Splitter::do_EVE2_TDC,    &Splitter::do_EVE2_EVE},
        {&Splitter::do_EVE3_TDC,    &Splitter::do_EVE3_EVE}, 
        {&Splitter::do_UNKN_TDC,    &Splitter::do_UNKN_EVE}
    };

    Input get_input_type(int nCol);

    void next(int n_col);

    void show_FSM();

    void write_event_data_line();

    void open_event_data_file(int i_valve, int i_laser);

    void start_new_event_data_file(int i_valve, int i_laser);

    long count_valves;
    long count_lasers;
    long count_events;
    long tdc_count;

    long count_lines;
    long count_lasers_all;
    long count_events_all;

    std::vector<long> arr_count_events;

    std::filesystem::path res_dir;


    std::ifstream file_src_data;
    // output result data
    std::ofstream file_res_info;
    std::ofstream file_res_data;

    State state, state_old;
    double data[4];
    double tdc_time;
};
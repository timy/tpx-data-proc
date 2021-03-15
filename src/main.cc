#include <iostream>
#include "tpx3parser.h"
#include "splitter.h"
#include "cluster.h"
#include "directory.h"

void inquire(const char* msg) {
    char c;
    cout << msg << " Do you really want to proceed? [y/n]";
    while (cin >> c) {
        switch (c) {
        case 'y': return;
        case 'n': exit(-1);
        default:
            cout << "please press '[y]es' or '[n]o'" << endl;
        }
    }
}

int main(int argc, char* argv[]) {

	if (argc == 1) {
		std::cout << "Argument of .tpx3 file name is required." << std::endl << std::endl;
		return -1;
	}

    Directory dir_res_root("res"); // directory to store results

    std::cout << "========================================" << std::endl;
    std::cout << "Step 0: convert .tpx3 file to ascii file" << std::endl;
    Directory dir_res_step1(dir_res_root.path() / "step1");
    string filename_tpx3 = string(argv[1]);
    string filename_converted = dir_res_step1.name() + "/converted.dat";
	TPX3Parser parser(filename_tpx3.c_str(), filename_converted.c_str());
	parser.process();

    std::cout << "========================================" << std::endl;
	std::cout << "Step 1: split the single ascii file into small files" << std::endl;
    Directory dir_res_step2(dir_res_root.path() / "step2");
    //if (!dir_res_step2.isNew())
    //    inquire("Found res_dir exist.");
    Splitter split(filename_converted.c_str(), dir_res_step2.path());
    split.process();
    
    std::cout << "========================================" << std::endl;
    std::cout << "Step 2: clusterize data" << std::endl;
    Directory dir_res_step3(dir_res_root.path() / "step3");
    string filename_clst_inp = dir_res_step2.name() + "/data_000000_000000.dat";
    string filename_clst_out = dir_res_step3.name() + "/clst_000000_000000.dat";
    Clusterizer cl(filename_clst_inp.c_str());
    cl.process();
    cl.dump_to_file(filename_clst_out.c_str());
    // clf.check();

    std::cout << "Hasta la vista!" << std::endl;

    return 0;
}
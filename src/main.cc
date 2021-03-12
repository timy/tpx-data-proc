#include <iostream>
#include "tpx3parser.h"
#include "splitter.h"
#include "cluster.h"

int main(int argc, char* argv[]) {

	if (argc == 1) {
		std::cout << "Argument of .tpx3 file name is required." << std::endl << std::endl;
		return -1;
	}

    const char* filename_step_0 = argv[1];
    const char* filename_step_1 = "converted.dat";

    std::cout << "========================================" << std::endl;
    std::cout << "Step 0: convert .tpx3 file to ascii file" << std::endl;
	TPX3Parser parser(filename_step_0, filename_step_1);
	parser.process();
    
    std::cout << "========================================" << std::endl;
	std::cout << "Step 1: split the single ascii file into small files" << std::endl;
    Splitter split(filename_step_1);
    split.process();
    
    std::cout << "========================================" << std::endl;
    std::cout << "Step 2: clusterize data" << std::endl;
    Clusterizer clf("res/data_000000_000000.dat");
    clf.process();
    clf.dump_to_file("test.dat");
    // clf.check();

    std::cout << "Hasta la vista!" << std::endl;

    return 0;
}
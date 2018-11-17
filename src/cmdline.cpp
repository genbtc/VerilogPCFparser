/* (C) 2018 - genBTC, All Rights Reserved */
/* November 17, 2018 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "main.h"

bool VERBOSE_V_MODE = 0;
bool PCF_SORT_ON = 0;

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

char* getCmdOption(char** begin, char** end, const std::string& option) {
    char** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
        return *itr;
    return 0;
}

bool commandLineOptionsHandler(int argc, char** argv) {
    //command line interpreter
    if (argc > 1) {
        if (cmdOptionExists(argv, argv + argc, "-f"))
            pcffile = getCmdOption(argv, argv + argc, "-f");
        VERBOSE_V_MODE = cmdOptionExists(argv, argv + argc, "-v");
        PCF_SORT_ON = cmdOptionExists(argv, argv + argc, "-s");
        if (cmdOptionExists(argv, argv + argc, "-h")) {
            std::cout << "Help file:\n";
            std::cout << "usage: " << argv[0] << " -f filename.pcf [opens PCF FILE for input]\n";
            std::cout << "usage: " << argv[0] << " -v [VERBOSE] (prints valid pin checks, etc)\n";
            std::cout << "usage: " << argv[0] << " -s [SORT] (sort pcf by pin #, ascending. print output)\n";
            std::cout << "usage: " << argv[0] << " -h [HELP] (this. lists command line usage options)\n";
            return true;
        }
    }
    return false;
}

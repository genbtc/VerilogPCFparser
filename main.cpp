/* (C) 2018 - genBTC, all rights reserved */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

#define TEST_PRINT_PCFREAD_CHECK 0
#define TEST_PRINT_VERILOG_CHECK 1
int VERBOSE_V_MODE = 0;
//default filenames
const char* verilogfile = "verilogtest.v";
const char* pcffile = "blackice-ii.pcf";
const char* badpcf  = "blackice-iii.pcf";

struct PCFlayout {
    std::string setio;
    std::string pinName;
    std::string pinNum;
    std::string comment;
};

std::vector<PCFlayout> parsePCF(const char* pcffile) {
    std::vector<PCFlayout> v;
    std::ifstream input(pcffile);              // open the file    
    std::string line;                          // iterate each line
    while (std::getline(input, line)) {        // getline returns the stream by reference, so this handles EOF
        std::stringstream ss(line);            // create a stringstream out of each line
        PCFlayout PCF_node;                    // start a new node
        while (ss) {                           // while the stream is good
            std::string word;                  // get first word
            if (ss >> word) {                  // if first word is set_io
                if (word.find("set_io") == 0) {
                    PCF_node.setio = word;  //not needed to store, but why not.
                    ss >> PCF_node.pinName >> PCF_node.pinNum;
                    //no break statement, means keep checking (next iteration, for comments)
                }
                else if (word[0] == '#') { // if it's a comment
                    int commentpos = line.find("#");
                    //if its not at the beginning of the line, store it
                    if (commentpos != 0)
                        PCF_node.comment = line.substr(commentpos);
                    break;  //or ignore the full line comment and move on
                }
                else {
                    std::cerr << "Unexpected symbol: '" << word << "'\n"; // report unexpected data
                    break; //and move onto next line. without this, it will accept more following values on this line
                }
            }
        }
        v.push_back(PCF_node);
    }
    return v;
}

struct Veriloglayout {
    std::string inpout;
    std::string bitfield;
    std::string pinName;
    std::string comment;
};

std::vector<Veriloglayout> parseVerilog(const char* verilogfile) {
    std::vector<Veriloglayout> v;
    std::ifstream input(verilogfile);           // open the file    
    std::string line;                          // iterate each line
    while (std::getline(input, line)) {        // getline returns the stream by reference, so this handles EOF
        std::stringstream ss(line);            // create a stringstream out of each line
        Veriloglayout VLog_node;                    // start a new node
        while (ss) {                           // while the stream is good
            std::string word;                  // get first word
            if (ss >> word) {                  // if first word is set_io
                if (word.find("input") == 0 || word.find("output") == 0 || word.find("inout") == 0) {
                    VLog_node.inpout = word;
                    if (ss >> word) {
                        if (word.find('[') == 0) {
                            VLog_node.bitfield = word;
                            ss >> VLog_node.pinName;
                        }
                        else
                            VLog_node.pinName = word;
                    }
                }
                else if (word[0] == '#') { // if it's a comment
                    int commentpos = line.find("#");
                    //if its not at the beginning of the line, store it
                    if (commentpos != 0)
                        VLog_node.comment = line.substr(commentpos);
                    break;  //or ignore the full line comment and move on
                }
                else {
                    std::cerr << "Unexpected symbol: '" << word << "'\n"; // report unexpected data
                    break; //and move onto next line. without this, it will accept more following values on this line
                }
            }
        }
        v.push_back(VLog_node);
    }
    return v;
}

bool hasDuplicatePinErrorsMap(std::vector<PCFlayout> &v1) {
    bool hasdupes{ false }; int dupes_found = 0;
    std::unordered_map<int, PCFlayout> u_map;    

    for (size_t i = 0; i < v1.size(); ++i) {
        PCFlayout vi = v1.at(i);
        if (vi.pinNum == "") continue;
        int pi = std::stoi(vi.pinNum);
        //Check map for duplicate
        if (u_map.find(pi) != u_map.end()) {
            hasdupes = true; ++dupes_found;
            std::cout << "Duplicate Pin: " << vi.pinNum << " = " << vi.pinName << " <-- Re-definition error.\n>Original Pin: "
              << u_map[pi].pinNum << " = " << u_map[pi].pinName << "\n";
            continue;
        }
        u_map[pi] = vi;
        //TODO -v verbose mode
        if (VERBOSE_V_MODE)
            std::cout << "Checking: " << vi.pinNum << " = " << vi.pinName << "\n";
    }
    if (hasdupes || dupes_found)
        std::cout << "\n" << dupes_found << " Duplicates Found\n";
    return hasdupes;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char **itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
        return *itr;
    return 0;
}

int main(int argc, char** argv) {
    //can provide filename as command line parameter so: PCFParser.exe -f blackice-iii.pcf
    //command line interpreter
    if (argc > 1) {
        if (cmdOptionExists(argv, argv + argc, "-f"))
            pcffile = getCmdOption(argv, argv + argc, "-f");
        VERBOSE_V_MODE = cmdOptionExists(argv, argv + argc, "-v");
        if (cmdOptionExists(argv, argv + argc, "-h")) {
            std::cout << "Help file:\n";
            std::cout << "usage: " << argv[0] << "-f filename.pcf [opens PCF FILE for input]\n";
            std::cout << "usage: " << argv[0] << "-v [VERBOSE mode prints valid pin checks also]\n";
            std::cout << "usage: " << argv[0] << "-h [HELP] (lists command line options)\n";
            return 0;
        }
    }

    std::cout << "Reading Input PCF File: " << pcffile << "\n";
    std::vector<PCFlayout> pcfnodes = parsePCF(pcffile);

    //visually prints input data we just read into the vector - to check validity, as a Unit Test
    if (TEST_PRINT_PCFREAD_CHECK) {
        std::cout << "Printing Parsed PCF Results:\n";
        for (auto node : pcfnodes) {
            if (node.pinName.length() != 0)
                std::cout << node.setio << " " << node.pinName << " " << node.pinNum << " " << node.comment << std::endl;
        }
        std::cout << "\n";
    }

    std::cout << "Checking for duplicate pins...\n";
    auto result = hasDuplicatePinErrorsMap(pcfnodes);
    std::cout << (result ? "Errors!" : "All OK!") << "\n";

    std::cout << "Reading Input Verilog File: " << verilogfile << "\n";
    std::vector<Veriloglayout> vlognodes = parseVerilog(verilogfile);
    
    //visually prints verilog data we just read into the vector - to check validity, as a Unit Test
    if (TEST_PRINT_VERILOG_CHECK) {
        std::cout << "Printing parsed Verilog:\n";
        for (auto node : vlognodes) {
            if (node.pinName.length() != 0)
                std::cout << node.inpout << ": " << node.pinName << " Bits:" << node.bitfield << " #" << node.comment << std::endl;
        }
        std::cout << "\n";
    }


    return result;
}

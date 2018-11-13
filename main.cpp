/* (C) 2018 - genBTC, All Rights Reserved */
/* November 12, 2018 */
/* "PCFParser by genBTC" - for drox's FPGA project. */
/* Reads some .PCF files and some .V verilog files and parses them */
/* Checks PCF file for duplicate pins */
/* Matches PCF file pins to Verilog module pin names, detect mismatches */
   /*
*/ /* PCF File lines are in the form of:
        set_io DBG1       49 # DBG1
*/ /* Verilog File lines are in the form of: 
        output RAMUB,
        inout  [55:0] PMOD,
*/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include "main.h"

#define TEST_PRINT_PCFREAD_CHECK 1
#define TEST_PRINT_VERILOG_CHECK 1
#define TEST_PCF_PIN_COUNT 1
int VERBOSE_V_MODE = 0;
//default filenames
const char* verilogfile = "verilogtest.v";
const char* verilognoisy = "verilogALL.v";
const char* pcffile = "blackice-ii.pcf";
const char* badpcf  = "blackice-iii.pcf";


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
                    int pi = std::stoi(PCF_node.pinNum);
                    PCF_node.pinNumInt = pi;
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
                    std::cerr << "Unresolved Symbol: '" << word << "'\n"; // report unexpected data
                    break; //and move onto next line. without this, it will accept more following values on this line
                }
            }
        }
        v.push_back(PCF_node);
    }
    return v;
}


std::vector<Veriloglayout> parseVerilog(const char* verilogfile) {
    std::vector<Veriloglayout> v;
    std::ifstream input(verilogfile);           // open the file    
    std::string line;                          // iterate each line
    while (std::getline(input, line)) {        // getline returns the stream by reference, so this handles EOF
        std::stringstream ss(line);            // create a stringstream out of each line
        Veriloglayout VLog_node;               // start a new node
        while (ss) {                           // while the stream is good
            std::string word;                  // get first word
            if (ss >> word) {                  // if first word is set_io
                if (word.find("input") == 0 || word.find("output") == 0 || word.find("inout") == 0) {
                    VLog_node.inpout = word;
                    if (ss >> word) {
                        if (word.find('[') == 0) {
                            VLog_node.bitfield = word;
                            VLog_node.hibit = std::stoi(word.substr(1, word.find(':')));
                            VLog_node.lobit = std::stoi(word.substr(word.find(':')+1,word.size()));
                            VLog_node.bits = VLog_node.hibit - VLog_node.lobit + 1;
                            ss >> VLog_node.pinName;
                        }
                        else
                            VLog_node.pinName = word;
                        //remove the ending comma
                        auto l = VLog_node.pinName.size() - 1;
                        if (l == VLog_node.pinName.find(','))
                            VLog_node.pinName.erase(l);
                    }
                    break;
                }
                else if (word.find("wire") == 0)  //marks a wire block.
                    break;
                else if (word[0] == '#') { // if it's a comment
                    int commentpos = line.find("#");
                    //if its not at the beginning of the line, store it
                    if (commentpos != 0)
                        VLog_node.comment = line.substr(commentpos);
                    break;  //or ignore the full line comment and move on
                }
                else if (word.find("verilog") == 0)  //marks the start of the file
                    break;
                else if (word.find("module") == 0) { //marks the start of the module definition
                    ss >> word; //this is the title of the module
                    break;
                }
                else if (word.find(");") == 0)  //marks the end of the file
                    break;
                else {
                    //noisy parser errors
                    std::cerr << "Error @ line: " << line << "\n";
                    std::cerr << "Unresolved Symbol: '" << word << "'\n";
                    break; //and move onto next line. without this, it will accept more following values on this line
                }
            }
        }
        v.push_back(VLog_node);
    }
    return v;
}

bool hasDuplicatePinErrorsMap(std::vector<PCFlayout> &v1, std::map<int, PCFlayout> &i_map) {
    bool hasdupes{ false }; int dupes_found = 0;

    for (auto vi : v1) { 
        if (vi.pinNum == "") continue;
        int pi = vi.pinNumInt;
        //Check map for duplicate
        if (i_map.find(pi) != i_map.end()) {
            hasdupes = true; ++dupes_found;
            std::cout << "Duplicate Pin: " << vi.pinNum << " = " << vi.pinName << " <-- Re-definition error.\n>Original Pin: "
              << i_map[pi].pinNum << " = " << i_map[pi].pinName << "\n";
            continue;
        }
        i_map[pi] = vi;
        if (VERBOSE_V_MODE)
            std::cout << "Checking: " << vi.pinNum << " = " << vi.pinName << "\n";
    }
    if (hasdupes || dupes_found)
        std::cout << "\n" << dupes_found << " Duplicates Found\n";
    return hasdupes;
}



bool comparePCFtoVerilog(std::vector<PCFlayout> &v1, std::vector<Veriloglayout> &v2, 
                         std::map<std::string, int> &pinBitNums) {
    //count up the number of pcf pins named the same thing
    for (auto node : v1) {
        if (node.pinName == "") continue;
        //strip out the [ ] 
        auto l = node.pinName.find('[');
        if (l != std::string::npos) {
            int pinbit = std::stoi(node.pinName.substr(l+1, node.pinName.find(']') - 1));
            //std::cout << "pinbit " << pinbit << std::endl;
            node.pinNameBit = pinbit;
            node.pinName.erase(l);
        }
        //make a map of the verilog to find by name and get the bits
        pinBitNums[node.pinName]++;  //increment seen pin bit count
    }
    //check output:
    //pinBitNum will be PINNAME , TOTALBITSACCOUNTEDFOR
    if (TEST_PCF_PIN_COUNT) {
        for (auto pin : pinBitNums) {
            std::cout << pin.first << " " << pin.second << "\n";
        }
    }
    std::cout << "\nComparing parsed_PCF with parsed_Verilog:\n";
    bool hasMismatches{ false }; int mismatches_found = 0;
    //O(n^2)? = meh
    for (auto pin : pinBitNums) {
        for (auto vnode : v2) {
            if (vnode.pinName == "") continue;
            if (pin.first.find(vnode.pinName) == 0) {
                if (pin.second != vnode.bits) {
                    std::cout << "Verilog bit-count " << vnode.pinName << " " << vnode.bits << "\n";
                    std::cout << " NOT EQUAL to: \n";
                    std::cout << "PCFfile bit-count " << pin.first << " " << pin.second << "\n";
                    hasMismatches = true;  mismatches_found++;
                    break;
                }
            }
        }
    }
    if (hasMismatches || mismatches_found)
        std::cout << "\n" << mismatches_found << " Mis-Matches Found\n";
    return hasMismatches;
}

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
        if (cmdOptionExists(argv, argv + argc, "-h")) {
            std::cout << "Help file:\n";
            std::cout << "usage: " << argv[0] << " -f filename.pcf [opens PCF FILE for input]\n";
            std::cout << "usage: " << argv[0] << " -v [VERBOSE] (prints valid pin checks, etc)\n";
            std::cout << "usage: " << argv[0] << " -h [HELP] (this. lists command line usage options)\n";
            return true;
        }
    }
    return false;
}

int main(int argc, char** argv) {
    //can provide filename as command line parameter so: PCFParser.exe -f blackice-iii.pcf -v
    if (auto cmdh = commandLineOptionsHandler(argc, argv))
        return cmdh;

    std::cout << "Reading Input PCF File: " << pcffile << "\n";
    std::vector<PCFlayout> pcfnodes = parsePCF(pcffile);
    
    //TODO make this a cmd line option:
    // sorts by pinNumInt ascending:
    std::cout << "Re-Sorting vector of PCF nodes by their arbitrary pin number, ascending\n";
    std::sort(pcfnodes.begin(), pcfnodes.end(), [](PCFlayout a, PCFlayout b) {return a.pinNumInt < b.pinNumInt; });

    //visually prints PCF input data we just read into the vector - to check validity, as a Unit Test
    if (TEST_PRINT_PCFREAD_CHECK) {
        std::cout << "Printing Parsed PCF:\n";
        for (auto node : pcfnodes) {
            if (node.pinName.length() != 0)
                std::cout << node.setio << " " << node.pinName << " " << node.pinNum << " " << node.comment << std::endl;
        }
        std::cout << "\n";
    }

    std::cout << "Checking for duplicate pins...\n";
    std::map<int, PCFlayout> i_map;
    auto result1 = hasDuplicatePinErrorsMap(pcfnodes, i_map);
    std::cout << (result1 ? "Errors!" : "All OK!") << "\n\n";

    std::cout << "Reading Input Verilog File: " << verilogfile << "\n";
    std::vector<Veriloglayout> vlognodes = parseVerilog(verilogfile);
    
    //visually prints Verilog data we just read into the vector - to check validity, as a Unit Test
    if (TEST_PRINT_VERILOG_CHECK) {
        std::cout << "Printing parsed Verilog:\n";
        for (auto node : vlognodes) {
            if (node.pinName.length() != 0) {
                std::cout << node.inpout << ": " << node.pinName;
                if (node.bits > 1)
                    std::cout << "  bit-count: " << node.bits;
                std::cout << "  " << node.comment << std::endl;
            }
        }
        std::cout << "\n";
    }
    
    //Start comparing verilog and PCF files together 
    std::map<std::string, int> pinBitNums;
    auto result2 = comparePCFtoVerilog(pcfnodes, vlognodes, pinBitNums);

    return (result1 || result2);
}


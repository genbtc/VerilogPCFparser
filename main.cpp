/* (C) 2018 - genBTC, all rights reserved */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

const char* verilogfile = "verilogtest.v";
const char* pcffile = "blackice-ii.pcf";

struct PCFlayout {
    std::string setio;
    std::string pinName;
    std::string pinNum;
    std::string comment;
};

std::vector<PCFlayout> parsePCF(const char* pcffile) {
    std::ifstream input(pcffile);
    std::vector<PCFlayout> v;           // a vector of PCFlayouts
    std::string line;
    while (std::getline(input, line)) {        // getline returns the stream by reference, so this handles EOF
        std::stringstream ss(line);            // create a stringstream out of line
        PCFlayout PCF_node;
        while (ss) {                           // while the stream is good
            std::string word;
            if (ss >> word) {                  // if there's still data to get
                if (word[0] == '#') { // if it's a comment
                    int commentpos = line.find("#");
                    //if its not at the beginning of the line, store it
                    if (commentpos != 0)
                        PCF_node.comment = line.substr(commentpos);
                    break;  //or ignore the full line comment and move on
                }
                else if (word.find("set_io") == 0) {
                    PCF_node.setio = word;  //not needed to store, but why not.
                    ss >> PCF_node.pinName >> PCF_node.pinNum;
                    //no break statement, means keep checking (next iteration, for comments)
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

bool hasDuplicatePinErrorsMap(std::vector<PCFlayout> &v1) {
    bool hasdupes{ false };
    std::unordered_map<int, PCFlayout> u_map;
    int dupes_found = 0;

    for (size_t i = 0; i < v1.size(); ++i)
    {
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
        //std::cout << "Checking: " << vi.pinNum << " = " << vi.pinName << "\n";
    }
    //if (hasdupes || dupes_found)
    std::cout << "\n" << dupes_found << " Duplicates Found\n";
    return hasdupes;
}

int main(int argc, char** argv) {
    if (argc > 1) {
        pcffile = argv[1];
    }
    std::cout << "Reading Input File: " << pcffile << "\n";
    std::vector<PCFlayout> pcfnodes = parsePCF(pcffile);

    std::cout << "Printing Parsed Results!\n";
    for (auto node : pcfnodes) {
        if (node.pinName.length() != 0)
            std::cout << node.setio << " " << node.pinName << " " << node.pinNum << " " << node.comment << std::endl;
    }
    std::cout << "\n";
    std::cout << "Checking for duplicate pins...\n";
    auto result = hasDuplicatePinErrorsMap(pcfnodes);
    std::cout << (result ? "Errors!" : "OK") << "\n";
    return result;
}

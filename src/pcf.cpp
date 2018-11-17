/* (C) 2018 - genBTC, All Rights Reserved */
/* November 17, 2018 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include "main.h"


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


bool hasDuplicatePinErrorsMap(std::vector<PCFlayout> &v1, std::map<int, PCFlayout> &pcfMap) {
    bool hasdupes{ false }; int dupes_found = 0;

    for (auto vi : v1) {
        if (vi.pinNum == "") continue;
        int pi = vi.pinNumInt;
        //Check map for duplicate
        if (pcfMap.find(pi) != pcfMap.end()) {
            hasdupes = true; ++dupes_found;
            std::cout << "Duplicate Pin: " << vi.pinNum << " = " << vi.pinName << " <-- Re-definition error.\n";
            std::cout << ">Original Pin: " << pcfMap[pi].pinNum << " = " << pcfMap[pi].pinName << "\n";
            continue;
        }
        pcfMap[pi] = vi;
        if (VERBOSE_V_MODE)
            std::cout << "Checking: " << vi.pinNum << " = " << vi.pinName << "\n";
    }
    if (hasdupes || dupes_found)
        std::cout << "\n" << dupes_found << " Duplicates Found\n";
    return hasdupes;
}

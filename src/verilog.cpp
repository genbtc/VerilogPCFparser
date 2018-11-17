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

#define TEST_PCFMAP_PIN_COUNT 0

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
                            VLog_node.lobit = std::stoi(word.substr(word.find(':') + 1, word.size()));
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


bool comparePCFtoVerilog(std::vector<PCFlayout> &v1, std::vector<Veriloglayout> &v2,
                         std::map<std::string, int> &pinBitNumsMap) {
    std::cout << "\nCount up the number of PCF pins with the same name (extract and remove the bitfield).\n";
    for (auto &pNode : v1) {
        if (pNode.pinName == "") continue;
        //strip out the [ ] 
        auto l = pNode.pinName.find('[');
        if (l != std::string::npos) {
            int pinbit = std::stoi(pNode.pinName.substr(l + 1, pNode.pinName.find(']') - 1));
            //std::cout << "pinbit " << pinbit << std::endl;
            pNode.pinNameBit = pinbit;
            pNode.pinNameBase = pNode.pinName;
            pNode.pinNameBase.erase(l);
            //make a map of the verilog to find by name and get the bits
            pinBitNumsMap[pNode.pinNameBase]++;  //increment seen pin bit count
        }
    }
    //check output: pinBitNum will be (PINNAME, TOTALBITSACCOUNTEDFOR)
    if (TEST_PCFMAP_PIN_COUNT) {
        for (auto pin : pinBitNumsMap) {
            std::cout << pin.first << " " << pin.second << "\n";
        }
    }
    std::cout << "\nComparing parsed_PCF with parsed_Verilog:\n";
    bool hasMismatches{ false }; int mismatches_found = 0;
    //O(n^2)? = meh
    for (auto pin : pinBitNumsMap) {
        for (auto vNode : v2) {
            if (vNode.pinName == "") continue;
            if (pin.first.find(vNode.pinName) == 0) {
                if (pin.second != vNode.bits) {
                    std::cout << "Verilog bit-count: " << vNode.pinName << " " << vNode.bits << "\n";
                    std::cout << " NOT EQUAL to: \n";
                    std::cout << "PCFfile bit-count: " << pin.first << " " << pin.second << "\n";
                    hasMismatches = true;  mismatches_found++;
                    break;
                }
            }
        }
    }
    if (hasMismatches || mismatches_found)
        std::cout << "\n" << mismatches_found << " Mis-Matches Found!\n";

    std::cout << "\nComparing parsed_PCF pin name bit number with parsed_Verilog bit field:\n";
    std::cout << "Finds errors where pin bit name is less than or greater than than the Verilog.v bit-field\n";
    //Finds Bit-Range errors between .PCF and .V
    for (auto pNode : v1) {
        if (pNode.pinName == "") continue;
        for (auto vNode : v2) {
            if (vNode.pinName == "") continue;
            if (pNode.pinName.find(vNode.pinName) == 0) {
                if (pNode.pinNameBit < vNode.lobit) {
                    std::cout << "Error: " << pNode.pinName << " @ .PCF = " << pNode.pinNameBit << " < " << vNode.lobit << " @ .V \n";
                }
                else if (pNode.pinNameBit > vNode.hibit) {
                    std::cout << "Error: " << pNode.pinName << " @ .PCF = " << pNode.pinNameBit << " > " << vNode.hibit << " @ .V \n";
                }
            }
        }
    }

    return hasMismatches;
}
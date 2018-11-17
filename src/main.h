#pragma once
/* (C) 2018 - genBTC, All Rights Reserved */
/* November 17, 2018 */

struct Veriloglayout {
    std::string inpout;
    std::string bitfield;
    int bits = 1;
    int hibit = INT_MAX;
    int lobit = 0;
    std::string pinName;
    std::string comment;
};

struct PCFlayout {
    std::string pinName;        //with the [7:0] bitfield(maybe)
    int pinNameBit = 0;
    std::string pinNameBase;    //without the bitfield
    std::string pinNum;
    int pinNumInt;
    std::string comment;
};

//Globals
extern bool VERBOSE_V_MODE;
extern bool PCF_SORT_ON;
extern const char* verilogfile;
extern const char* verilognoisy;
extern const char* pcffile;
extern const char* badpcf;

//from main.cpp
int main(int argc, char ** argv);
void printParsedVerilogCheck(std::vector<Veriloglayout> &vlognodes);
void printParsedPCFcheck(std::vector<PCFlayout> &pcfnodes);

//from pcf.cpp
std::vector<PCFlayout> parsePCF(const char* pcffile);
bool hasDuplicatePinErrorsMap(std::vector<PCFlayout> &v1, std::map<int, PCFlayout> &pcfMap);

//from verilog.cpp
std::vector<Veriloglayout> parseVerilog(const char* verilogfile);
bool comparePCFtoVerilog(std::vector<PCFlayout> &v1, std::vector<Veriloglayout> &v2, std::map<std::string, int> &pinBitNumsMap);

//from cmdline.cpp
bool cmdOptionExists(char** begin, char** end, const std::string& option);
char* getCmdOption(char** begin, char** end, const std::string& option);
bool commandLineOptionsHandler(int argc, char** argv);
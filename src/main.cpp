/* (C) 2018 - genBTC, All Rights Reserved */
/* November 12-17, 2018 */
/* "PCFParser by genBTC" - for drox's FPGA project. */
/* Step 1: Reads PCF file and parse it.  */
/*  Option  -s: Sort it by pin number    */
/*  Option -c1: Output .PCF file to screen */
/* Step 2: Checks PCF file for duplicate pins */
/* Step 3: Read Verilog file and parse it */
/*  Option -c2: Output .V file to screen  */
/* Step 4: Compare PCF file pins to Verilog module pin names, 
    detect mismatches, and bit-range errors */
/* PCF File lines are in the form of:
        set_io DBG1       49 # DBG1
   Verilog File lines are in the form of: 
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


//default filenames
const char* verilogfile = "testdata/verilogtest.v";
const char* verilognoisy = "testdata/verilogALL.v";
const char* pcffile = "testdata/blackice-ii.pcf";
const char* badpcf = "testdata/blackice-iii.pcf";


int main(int argc, char** argv) {
    //can provide filename as command line parameter so: PCFParser.exe -f blackice-iii.pcf -v
    if (auto cmdh = commandLineOptionsHandler(argc, argv))
        return cmdh;
//Step 1:
    //Parser: PCF
    std::cout << "Reading Input PCF File: " << pcffile << "\n";
    std::vector<PCFlayout> pcfnodes = parsePCF(pcffile);
    //Check if we read anything, if not, error out.
    if (pcfnodes.size() < 1) { std::cerr << "Failed to parse anything in the PCF file. Exiting!\n"; return -1; }
    //Option -s = Sort
    if (PCF_SORT_ON) {
        std::cout << "Re-Sorting vector of PCF nodes by their given pin number, ascending.\n";
        std::sort(pcfnodes.begin(), pcfnodes.end(), [](PCFlayout a, PCFlayout b) { return a.pinNumInt < b.pinNumInt; });
    }
    //outputs parsed PCF
    printParsedPCFcheck(pcfnodes);

//Step 2:
    //Dupe Check on PCF
    std::cout << "Checking for duplicate pins...\n";
    std::map<int, PCFlayout> pcfMap;
    auto result1 = hasDuplicatePinErrorsMap(pcfnodes, pcfMap);
    std::cout << (result1 ? "Errors!" : "All OK!") << "\n\n";

//Step 3:
    //Parser: Verilog
    std::cout << "Reading Input Verilog File: " << verilogfile << "\n";
    std::vector<Veriloglayout> vlognodes = parseVerilog(verilogfile);
    //Check if we read anything, if not, error out.
    if (vlognodes.size() < 1) { std::cerr << "Failed to parse anything in the Verilog file. Exiting!\n"; return -2; }
    //outputs parsed verilog
    printParsedVerilogCheck(vlognodes);

//Step 4:
    //Compare Verilog and PCF files together and checks:
    auto result2 = comparePCFtoVerilog(pcfnodes, vlognodes);
    std::cout << (result2 ? "Errors!" : "All OK!") << "\n\n";

    return (result1 || result2);
}

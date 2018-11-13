#pragma once
struct Veriloglayout {
    std::string inpout;
    std::string bitfield;
    int bits = 1;
    int hibit = 0;
    int lobit = 0;
    std::string pinName;
    std::string comment;
};

struct PCFlayout {
    std::string setio;
    std::string pinName;
    int pinNameBit;
    std::string pinNum;
    int pinNumInt;
    std::string comment;
};

int main(int argc, char ** argv);
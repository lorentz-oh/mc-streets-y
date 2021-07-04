#pragma once
#include <vector>
#include "region.h"
#include "block.h"

//class for schematics
class Schematic
{
public:
    Schematic();
    void Read(std::ifstream &Is);
    void Put(Region &Reg, int X, int Y, int Z) ;
    //Puts the schematic into world only if it fully fits
    //at the specified position
    void PutFull(Region &Reg, int X, int Y, int Z);
private:
    std::vector<BlockIndexed> Blocks;
    int Length = 0;
    int Width = 0;
    int Height = 0;
    int Volume = 0;
    //The coordinates of schematic's center
    int CX, CZ;
};

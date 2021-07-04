#pragma once
#include <memory>
#include <fstream>
#include "section.h"
#include "tag_compound.h"

using SctnPtr = std::unique_ptr<Section>;
typedef std::pair<std::string, std::unique_ptr<nbt::tag_compound>> Tag;

class Chunk
{
public:
    //constructs a new, empty chunk, takes index
    //in the region file as argument
    Chunk(int Index, int RegX, int RegZ);
    //reads compressed chunk from stream
    Chunk(std::istream &Is);
    //Retrieve tag, containing chunks data structures
    void UpdateTag();
    //Reads "NBTTag", constructs necessary sections, sets blocks
    void ReadTag(nbt::io::stream_reader &StreamReader);
    //Writes the compressed chunk into ofstream, returns the number
    //of 4KiB sectors written. It also seeks so the ofstream is
    //at a loation divisible by 4096, for chunk padding
    int WriteChunk(std::ofstream &Ofs);
    void SetBlock(int X, int Y, int Z, char Id, char Add, char Data);
    //sets not only id, but data and add fields
    void SetBlockFull(int X, int Y, int Z, const Block& Block);
    char GetBlock(int X, int Y, int Z);
    Block GetBlockFull(int X, int Y, int Z);
    Tag ChunkTag;
private:
    std::array<SctnPtr,16> Sections;
};

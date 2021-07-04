#pragma once
#include <memory>
#include "chunk.h"
#include <fstream>

using ChunkPtr = std::unique_ptr<Chunk>;
//chunk's metadata like location in file, size, timestamp

class Region
{
public:
    Region(int X, int Z, const std::string& RegDir);
    void SetBlock(int X, int Y, int Z, char Id, char Add, char Data);
    //Checks bounds before setting block
    void SetBlockSafe(int X, int Y, int Z, Block &Block);
    char GetBlock(int X, int Y, int Z);
    Block GetBlockFull(int X, int Y, int Z);
    //saves changed region to disk
    void SaveRegion();
	bool Exists(){return m_Exists;}
private:
    //Reads region from the ifstream, puts chunks into the array of chunks
    //Also initializes non-existent chunks
    void ReadRegion(std::ifstream& Ifs);
    //Puts empty chunks into the chunks array
    void InitRegion();
    //Writes a chunk into output stream.
    //Returns number of 4KiB sectors written
    int WriteChunk();
    /***objects***/
	//true if region exists on the disk
	bool m_Exists;
    //region directory
    std::string RegDir;
    //region coordinates
    int X, Z;
    std::ifstream Ifs;
    std::array<ChunkPtr,1024> Chunks;
};

#include <fstream>
#include <string>
#include <math.h>
#include <iostream>
#include <chrono>
#include "region.h"
#include "io/stream_reader.h"
#include "io/stream_writer.h"
#include "io/izlibstream.h"
#include "io/ozlibstream.h"

class Location
{
public:
    u_int Offset;
    u_int Sectors;
    Location()
    {

    }

    Location(std::ifstream &Ifs)
    {
        u_char Loc[4];
        Ifs.read(reinterpret_cast<char*>(Loc), 4);
        Offset = Loc[0] * 256 * 256 + Loc[1] * 256 + Loc[2];
        Sectors = Loc[3];
    }

    void Write(std::ostream &Os)
    {
        u_char Loc[4];
        Loc[0] = Offset / 256 / 256;
        Loc[1] = (Offset / 256) % 256;
        Loc[2] = Offset % 256;
        Loc[3] = Sectors;
        Os.write(reinterpret_cast<char*>(Loc), 4);
    }
};

Region::Region(int X, int Z, const std::string& RegDir):
    X{X},
    Z{Z},
    RegDir{RegDir}
{
    std::string RegPath = RegDir;
    if(RegPath.back() != '/'){
        RegPath += '/';}
    RegPath += "r." + std::to_string(X) + '.' + std::to_string(Z) + ".mca";
    Ifs.open(RegPath, std::ios::binary);
    if(Ifs){
		m_Exists = true;
        ReadRegion(Ifs);}
    else{
		m_Exists = false;
        InitRegion();}
}

void Region::InitRegion()
{
    for (int i = 0; i < Chunks.size(); i++) {
        Chunks[i].reset(new Chunk(i,X,Z));
    }
}

void Region::ReadRegion(std::ifstream &Ifs)
{
    std::vector<Location> Locats;
    Locats.reserve(1024);
    for(int i = 0; i < 1024; i++){
        Locats.emplace_back(Ifs);
    }

    for(int i = 0; i < 1024; i++){
        if(Locats[i].Offset == 0){
            Chunks[i].reset(new Chunk{i,X,Z});
            continue;}
        zlib::izlibstream IZlibS{Ifs};
        Ifs.seekg(Locats[i].Offset * 4096 + 5);
        Chunks[i].reset(new Chunk(IZlibS));
    }
}

void WriteTimestamp(std::ostream &OStr)
{
    uint32_t Time = std::time(nullptr);
    char Timestamp[4];
    for(int i = 0; i < 4; i++){
        Timestamp[i] = reinterpret_cast<char*>(&Time)[3-i];
    }
    OStr.write(Timestamp, 4);
}

void Region::SaveRegion()
{
    std::string RegPath = RegDir;
    if(RegPath.back() != '/'){
        RegPath += '/';}
    RegPath += "r." + std::to_string(X) + '.' + std::to_string(Z) + ".mca";
    std::remove(RegPath.c_str());
    std::ofstream Ofs{RegPath};
    if(!Ofs){
        throw std::invalid_argument("Couldn't open file to write\n");}
    //skip header, it will be filled in later
    std::vector<Location> Locats(1024);

    if(Locats.size() != 1024){
        std::cout << "Vector size has unexpected value." << std::endl;
        throw std::invalid_argument("");}

    Ofs.seekp(8192);
    int Offset = 2;
    for(std::size_t i = 0; i < 1024; i++){
        int SectorCount = Chunks[i]->WriteChunk(Ofs);
        Locats[i].Offset = Offset;
        Locats[i].Sectors = SectorCount;
        Offset += SectorCount;
    }

    Ofs.seekp(0);
    for(auto Loct : Locats){
        Loct.Write(Ofs);
    }
    for(int i = 0; i < 1024; i++){
        WriteTimestamp(Ofs);
    }
}

char Region::GetBlock(int X, int Y, int Z)
{
    int ChunkX = X / 16;
    int ChunkZ = Z / 16;
    int LclX = X % 16;
    int LclZ = Z % 16;
    int Index = ChunkZ * 32 + ChunkX;
    return Chunks[Index]->GetBlock(LclX, Y, LclZ);
}

Block Region::GetBlockFull(int X, int Y, int Z)
{
    int ChunkX = X / 16;
    int ChunkZ = Z / 16;
    int LclX = X % 16;
    int LclZ = Z % 16;
    int Index = ChunkZ * 32 + ChunkX;
    return Chunks[Index]->GetBlockFull(LclX, Y, LclZ);
}

void Region::SetBlock(int X, int Y, int Z, char Id, char Add, char Data)
{
    int ChunkX = X / 16;
    int ChunkZ = Z / 16;
    int LclX = X % 16;
    int LclZ = Z % 16;
    int Index = ChunkZ * 32 + ChunkX;
    Chunks[Index]->SetBlock(LclX, Y, LclZ, Id, Add, Data);
}

void Region::SetBlockSafe(int X, int Y, int Z, Block &Block)
{
    if(X > 511 || X < 0 || Z > 511 || Z < 0 || Y > 255 || Y < 0){
        return;}
    int ChunkX = X / 16;
    int ChunkZ = Z / 16;
    int LclX = X % 16;
    int LclZ = Z % 16;
    int Index = ChunkZ * 32 + ChunkX;
    Chunks[Index]->SetBlockFull(LclX, Y, LclZ, Block);
}

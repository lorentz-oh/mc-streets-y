#include <iostream>
#include "ids.h"
#include "chunk.h"
#include "value.h"
#include "tag_list.h"
#include "tag_primitive.h"
#include "io/stream_reader.h"
#include "io/ozlibstream.h"
#include "io/izlibstream.h"
#include "text/json_formatter.h"

void GetEmptyChunk(nbt::tag_compound &ChunkTag)
{
    static bool WasRead = false;
    static Tag Chunk;
    if(!WasRead){
        std::ifstream Ifs{"chunk.nbt", std::ios::binary};
        if(!Ifs){
            throw std::invalid_argument("Couldn't read chunk example\n");
        }
        zlib::izlibstream IZlibS{Ifs};
        Chunk = nbt::io::read_compound(IZlibS);
        WasRead = true;
    }
    ChunkTag = *Chunk.second;
}

Chunk::Chunk(int Index, int RegX, int RegZ)
{
    ChunkTag.second.reset(new nbt::tag_compound);
    GetEmptyChunk(*ChunkTag.second);
    nbt::tag_compound &CompTag = *ChunkTag.second;
    int X = Index % 32 + RegX * 32; //region is 32 x 32 chunks
    int Z = Index / 32 + RegZ * 32;
    nbt::tag_compound& Level = CompTag["Level"].as<nbt::tag_compound>();
    nbt::tag_int& XPos = Level["xPos"].as<nbt::tag_int>();
    XPos.set(X);
    nbt::tag_int& ZPos = Level["zPos"].as<nbt::tag_int>();
    ZPos.set(Z);
}

Chunk::Chunk(std::istream &Is)
{
    ChunkTag = nbt::io::read_compound(Is);
    nbt::tag_compound &Tag = *ChunkTag.second;
    nbt::tag_compound &Level = Tag["Level"].as<nbt::tag_compound>();
    nbt::tag_list &Sctns = Level["Sections"].as<nbt::tag_list>();

    for(int i = 0; i<Sctns.size(); i++){
        nbt::tag_compound &Sctn = Sctns.at(i).as<nbt::tag_compound>();
        int Index = Sctn["Y"].as<nbt::tag_byte>().get();
        Sections[Index].reset(new Section(Sctn));
    }
    Sctns.clear();
}

void Chunk::UpdateTag()
{
    nbt::tag_compound &CompTag = *ChunkTag.second;
    nbt::tag_compound& Level = CompTag["Level"].as<nbt::tag_compound>();
    nbt::tag_list& Sctns = Level["Sections"].as<nbt::tag_list>();
    for(int i = 0; i < 16; i++){
        if(!Sections[i]){
            continue;}
        nbt::tag_compound Sctn = Sections[i]->GetTag(i);
        Sctns.push_back(nbt::value_initializer{Sctn.clone()});
    }
    nbt::tag_byte& Light = Level["LightPopulated"].as<nbt::tag_byte>();
    Light.set('\1');
}

void WriteChunkHeader(std::ostream &OutStream, u_int32_t ChunkSize)
{
    char BSize[4];
    char* p_Size = reinterpret_cast<char*>(&ChunkSize);

    for(int i = 0; i < 4; i++){
        BSize[i] = p_Size[3-i];
    }
    OutStream.write(BSize,4);
    OutStream.put('\2');
}

int Chunk::WriteChunk(std::ofstream &Ofs)
{
    UpdateTag();
    zlib::ozlibstream OZlibStrm{Ofs};

    std::streamoff Start = Ofs.tellp();
    Ofs.seekp(Start + 5);//skip size and compression type data of the chunk
    nbt::io::write_tag("",*ChunkTag.second,OZlibStrm);
    OZlibStrm.close();

    std::streamoff End = Ofs.tellp();
    u_int32_t ChkSize = End - Start;
    Ofs.seekp(Start);
    WriteChunkHeader(Ofs, ChkSize - 5);

    std::streamoff Padding = 4096 - End % 4096;
    //trick to pad the last chunk
    Ofs.seekp(End + Padding - 1);
    Ofs.put('\0');

    return ChkSize / 4096 + 1;
}

void Chunk::SetBlock(int X, int Y, int Z, char Id, char Add, char Data)
{
    int SctnY = Y / 16;
    int LclY = Y % 16;
    if(!Sections[SctnY]){
        Sections[SctnY].reset(new Section());}
    Sections[SctnY]->Blocks[LclY][Z][X].Data[BLOCK_ID] = Id;
    Sections[SctnY]->Blocks[LclY][Z][X].Data[BLOCK_ADD] = Add;
    Sections[SctnY]->Blocks[LclY][Z][X].Data[BLOCK_DATA] = Data;
}

void Chunk::SetBlockFull(int X, int Y, int Z, const Block &Block)
{

    std::size_t SctnY = Y / 16;
    int LclY = Y % 16;
    if(!Sections[SctnY]){
        Sections[SctnY].reset(new Section());}
    Sections[SctnY]->Blocks[LclY][Z][X] = Block;
}

char Chunk::GetBlock(int X, int Y, int Z)
{
    std::size_t SctnY = Y / 16;
    int LclY = Y % 16;
    if(!Sections[SctnY]){
        return MC_AIR;}
    return Sections[SctnY]->Blocks[LclY][Z][X].Data[BLOCK_ID];
}

Block Chunk::GetBlockFull(int X, int Y, int Z)
{
    std::size_t SctnY = Y / 16;
    int LclY = Y % 16;
    if(!Sections[SctnY]){
        return Block('\0', '\0', '\0');}
    return Sections[SctnY]->Blocks[LclY][Z][X];
}

#include "schematic.h"
#include "tag_compound.h"
#include "io/stream_reader.h"
#include "tag_primitive.h"
#include "tag_array.h"
#include "const.h"

Schematic::Schematic()
{

}

char GetNibble(nbt::tag_byte_array &Vec, int Index)
{
    char Byte = Vec[Index/2];
    if((Index % 2) == 0){
        Byte = (Byte >> 4) & 0x0f;
    }
    else {
        Byte = Byte & 0x0f;
    }
    return Byte;
}

void Schematic::Read(std::ifstream &Ifs)
{
    Blocks.clear();
    auto Pair = nbt::io::read_compound(Ifs);
    auto &Tag = *Pair.second;
    Length = Tag["Length"].as<nbt::tag_short>().get();
    Width = Tag["Width"].as<nbt::tag_short>().get();
    Height = Tag["Height"].as<nbt::tag_short>().get();
    Volume = Length * Width * Height;
    CX = Width / 2;
    CZ = Length / 2;

    auto BlocksVec = Tag["Blocks"].as<nbt::tag_byte_array>();
    auto DataVec = Tag["Data"].as<nbt::tag_byte_array>();
    bool AddPresent = Tag.has_key("AddBlocks");
    auto & AddVec = DataVec;
    if(AddPresent){
        AddVec = Tag["AddBlocks"].as<nbt::tag_byte_array>();}

    Blocks.reserve(Volume);
	char Id;
    for(int i = 0; i < BlocksVec.size(); ++i){
        Id = BlocksVec[i];
        //ignore air
        if(Id == '\0'){
            continue; }

        char Data = DataVec[i];
        char Add = '\0';
        if(AddPresent){
            Add = GetNibble(AddVec, i);}

        int Y = i/(Length*Width);
        int Z = i%(Length*Width)/Length;
        int X = i%Width;

        Blocks.emplace_back(Id,Data,Add,X,Y,Z);
    }
    Blocks.shrink_to_fit();
}

void Schematic::Put(Region &Reg, int X, int Y, int Z) 
{
    for(auto & Block : Blocks){
        Reg.SetBlockSafe(Block.X + X - CX,
                         Block.Y + Y,
                         Block.Z + Z - CZ,
                         Block);
    }
}

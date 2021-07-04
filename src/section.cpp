#include "section.h"
#include "tag_array.h"
#include "tag_primitive.h"

Section::Section(nbt::tag_compound &Sctn)
{
    FillWhole(Sctn["Blocks"].as<nbt::tag_byte_array>(), BLOCK_ID);
    FillHalf(Sctn["Data"].as<nbt::tag_byte_array>(), BLOCK_DATA);
    FillHalf(Sctn["BlockLight"].as<nbt::tag_byte_array>(), BLOCK_LIGHT);
    FillHalf(Sctn["SkyLight"].as<nbt::tag_byte_array>(), BLOCK_SKYLIGHT);

    //Add field may not exist
    if(Sctn.has_key("Add")){
        FillHalf(Sctn["Add"].as<nbt::tag_byte_array>(), BLOCK_ADD);}
    else{
        for(int Y = 0; Y < 16; Y++){
            for(int Z = 0; Z < 16; Z++){
                for(int X = 0; X < 16; X++){
                    Blocks[Y][Z][X].Data[BLOCK_ADD] = '\0';
                }
            }
        }
    }
}

Section::Section()
{
    for(int Y = 0; Y < 16; Y++){
        for(int Z = 0; Z < 16; Z++){
            for(int X = 0; X < 16; X++){
                Blocks[Y][Z][X].Data[BLOCK_ID] = '\0';
                Blocks[Y][Z][X].Data[BLOCK_ADD] = '\0';
                Blocks[Y][Z][X].Data[BLOCK_DATA] = '\0';
                Blocks[Y][Z][X].Data[BLOCK_LIGHT] = '\15';
                Blocks[Y][Z][X].Data[BLOCK_SKYLIGHT] = '\15';

            }
        }
    }
}

void Section::FillHalf(nbt::tag_byte_array Array, BlockField Field)
{
    int Y,Z,X;
    int Size = Array.size();
    if(Size != 2048){
        throw std::invalid_argument("Incorrect array size\n");}
    for(int i = 0; i < Size; ++i){
        Y = i/(16*8);
        Z = i%(16*8)/8;
        X = i%8*2;
        char Byte = Array[i];
        char First = Byte & 0x0f;
        char Second = (Byte >> 4) & 0x0f;
        Blocks[Y][Z][X].Data[Field] = First;
        Blocks[Y][Z][X+1].Data[Field] = Second;
    }
}

void Section::FillWhole(nbt::tag_byte_array Array, BlockField Field)
{
    int Y,Z,X;
    int Size = Array.size();
    if(Size != 4096){
        throw std::invalid_argument("Incorrect array size\n");}
    for(int i = 0; i < Size; ++i){
        Y = i/16/16;
        Z = (i/16)%16;
        X = i%16;
        Blocks[Y][Z][X].Data[Field] = Array[i];
    }
}

nbt::tag_byte_array Section::GetWhole(BlockField Field)
{
    nbt::tag_byte_array Array;
    auto Vec = Array.get();
    Vec.reserve(4096);
    int Y,Z,X;
    for(int i = 0; i < 4096; ++i){
        Y = i/16/16;
        Z = (i/16)%16;
        X = i%16;
        Array.push_back(Blocks[Y][Z][X].Data[Field]);
    }
    return std::move(Array);
}

nbt::tag_byte_array Section::GetHalf(BlockField Field)
{
    nbt::tag_byte_array Array;
    auto Vec = Array.get();
    Vec.reserve(2048);
    int Y,Z,X;
    for (int i = 0; i < 2048; i++) {
        Y = i/(16*8);
        Z = i%(16*8)/8;
        X = (i%8)*2;
        char First = Blocks[Y][Z][X].Data[Field];
        char Second = Blocks[Y][Z][X+1].Data[Field];
        Second <<= 4;
		First &= 0x0f;
        char Byte = First | Second;
        Array.push_back(Byte);
    }
    return std::move(Array);
}

signed char IntToChar(int I)
{
    u_char TmpUch = static_cast<u_char>(I);
    char RetCh = *reinterpret_cast<char*>(&TmpUch);
    return RetCh;
}

nbt::tag_compound Section::GetTag(int Index)
{
    nbt::tag_compound Section = {};
    nbt::tag_byte Y = IntToChar(Index);
    Section.insert("Y",nbt::value_initializer{Y});
    Section.insert("Blocks", nbt::value_initializer{GetWhole(BLOCK_ID)});
    Section.insert("Data", nbt::value_initializer{GetHalf(BLOCK_DATA)});
    Section.insert("Add", nbt::value_initializer{GetHalf(BLOCK_ADD)});
    Section.insert("BlockLight", nbt::value_initializer{GetHalf(BLOCK_LIGHT)});
    Section.insert("SkyLight", nbt::value_initializer{GetHalf(BLOCK_SKYLIGHT)});
    return Section;
}

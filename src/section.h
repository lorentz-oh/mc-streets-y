#pragma once
#include "block.h"
#include "tag_compound.h"

//minecraft.gamepedia.com/index.php?title=Chunk_format&oldid=1229175

class Section
{
public:
    //Constructor which constructs an empty section
    Section();
    //Constructs a section from nbt tag
    Section(nbt::tag_compound& Sctn);
    nbt::tag_compound GetTag(int Index);
    //Blocks, ordered YZX
    Block Blocks[16][16][16];
private:
    //fills blocks' fields, that are one byte in size (block id)
    void FillWhole(nbt::tag_byte_array Array, BlockField Field);
    //fills blocks' fields, that are just half a byte in size (data and add)
    void FillHalf(nbt::tag_byte_array Array, BlockField Field);
    //Gets tag of fields, which are one byte
    nbt::tag_byte_array GetWhole(BlockField Field);
    //Gets tag of half-byte fields
    nbt::tag_byte_array GetHalf(BlockField Field);
};

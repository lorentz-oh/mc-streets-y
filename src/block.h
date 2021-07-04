#pragma once
typedef unsigned char u_ch;
//these are enumerations for different fields in the block's data
enum BlockField
{
    BLOCK_ID,
    BLOCK_DATA,
    BLOCK_ADD,
    BLOCK_LIGHT,
    BLOCK_SKYLIGHT
};

class Block
{
public:
    Block();
    Block(char Id, char Data, char Add);
    char Data[5];
};

class BlockIndexed : public Block
{
public:
    BlockIndexed(char Id, char Data, char Add, int X, int Y, int Z);
    int X,Y,Z;
};

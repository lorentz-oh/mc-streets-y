#include "block.h"

Block::Block(){}

Block::Block(char Id, char Data, char Add)
{
    this->Data[BLOCK_ID] = Id;
    this->Data[BLOCK_DATA] = Data;
    this->Data[BLOCK_ADD] = Add;
}

BlockIndexed::BlockIndexed(char Id, char Data, char Add, int X, int Y, int Z):
    Block(Id,Data,Add),
    X{X},
    Y{Y},
    Z{Z}
{
}

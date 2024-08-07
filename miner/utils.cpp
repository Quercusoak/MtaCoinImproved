#include "utils.h"

void calculateHashcrc32(BLOCK_T& block, int difficulty)
{
    
    block.hash = crc32Hash(block);

    while (__builtin_clz(block.hash) != difficulty)
    {
        block.nonce++;
        block.hash = crc32Hash(block);
    } 
}

unsigned int crc32Hash(const BLOCK_T& block)
{
    unsigned int hash = crc32(0L, Z_NULL, 0);
    hash = crc32(hash, (const Bytef*)&block.height, sizeof(block.height));
    hash = crc32(hash, (const Bytef*)&block.timestamp, sizeof(block.timestamp));
    hash = crc32(hash, (const Bytef*)&block.prev_hash, sizeof(block.prev_hash));
    hash = crc32(hash, (const Bytef*)&block.nonce, sizeof(block.nonce));
    hash = crc32(hash, (const Bytef*)&block.relayed_by, sizeof(block.relayed_by));

    return hash;
}


void printBlockInfo(const BLOCK_T& g_block)
{
    printf("height = %d, timestamp = %d, hash = 0x%X, prev_hash = 0x%X, difficulty = %d, nonce = %d, relayed_by = #%d"
    ,g_block.height, g_block.timestamp, g_block.hash,g_block.prev_hash,g_block.difficulty,g_block.nonce, g_block.relayed_by);
}

BLOCK_T newServerBlockForMining(const BLOCK_T& prev)
{
    BLOCK_T new_block;
    new_block.height = prev.height +1;
    new_block.prev_hash = prev.hash;
    new_block.difficulty = prev.difficulty;

    return new_block;
}

void printErrorWrongPrevHash(const BLOCK_T& g_block, unsigned int prev_hash)
{
    printf("\nError - block invalid. Wrong prev_hash for block #%d by miner %d: recieved 0x%X but should be 0x%X\n"
    ,g_block.height, g_block.relayed_by, g_block.prev_hash, prev_hash);
}

void printErrorBadHash(const BLOCK_T& g_block, unsigned int calcHash)
{
    printf("\nError - block invalid. Wrong hash for block #%d by miner %d: recieved 0x%X but calculated 0x%X\n"
    ,g_block.height, g_block.relayed_by, g_block.hash, calcHash);
}
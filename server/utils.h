#ifndef __UTILS_H
#define __UTILS_H

#pragma once

#include <stdio.h>
#include "Block.h"
using namespace std;
#include <zlib.h>
#include <stdlib.h> 

const int MSG_TYPE_MINER_SUBSCRIPTION = 1;
const int MSG_TYPE_BLOCK_DATA = 2;

// calls crc32Hash and checks if num leading 0 equals difficulty, inc nonce
void calculateHashcrc32(BLOCK_T&, int);

// crc32 function on block fields: height, timestamp, prev_hash, nonce, relayed_by
unsigned int crc32Hash(const BLOCK_T&);

void printBlockInfo(const BLOCK_T&);

// new block with height, prev_hash and difficulty
BLOCK_T newServerBlockForMining(const BLOCK_T&);

void printErrorWrongPrevHash(const BLOCK_T&, unsigned int);

void printErrorBadHash(const BLOCK_T&, unsigned int);

#endif
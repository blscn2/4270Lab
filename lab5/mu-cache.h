#include "instructions.h"
/******************************************************************************/
/* CACHE STRUCTURE                                                            */
/******************************************************************************/
#define NUM_CACHE_BLOCKS 16
#define WORD_PER_BLOCK 4

#define GETINDEX(x) 	( x >> 4 ) & 0x0F
#define	GETTAG(x) 	( x >> 8 ) & 0xFFFFFF
#define GETBYTEOFF(x) 	( x & 0x03)
#define	GETBLOCKOFF(x) 	( x >> 2 ) & 0x03


typedef struct CacheBlock_Struct {

  int valid; //indicates if the given block contains a valid data. Initially, this is 0
  uint32_t tag; //this field should contain the tag, i.e. the high-order 32 - (2+2+4)  = 24 bits
  uint32_t words[WORD_PER_BLOCK]; //this is where actual data is stored. Each word is 4-byte long, and each cache block contains 4 blocks.
  
  
} CacheBlock;

typedef struct Cache_Struct {

  CacheBlock blocks[NUM_CACHE_BLOCKS]; // there are 16 blocks in the cache
  
} Cache;



/***************************************************************/
/* CACHE STATS                                                 */
/***************************************************************/
uint32_t cache_misses; //need to initialize to 0 at the beginning of simulation start
uint32_t cache_hits;   //need to initialize to 0 at the beginning of simulation start


/***************************************************************/
/* CACHE OBJECT                                                */
/***************************************************************/
Cache L1Cache; //need to use this in the simulator


/***************************************************************/
/* Configure cache for use                                       */
/***************************************************************/
void LoadCache(uint32_t address){
	uint8_t index = GETINDEX(address);
	uint32_t tag = GETTAG(address);

	uint32_t base_addr = address & 0xFFFFFFF0;
	
	L1Cache.blocks[index].valid = 1;
	L1Cache.blocks[index].tag = tag;

	L1Cache.blocks[index].words[0] = mem_read_32(base_addr);
	L1Cache.blocks[index].words[1] = mem_read_32(base_addr + 4);
	L1Cache.blocks[index].words[2] = mem_read_32(base_addr + 8);
	L1Cache.blocks[index].words[3] = mem_read_32(base_addr + 12);

}

uint8_t CheckInCache(uint32_t address){

}
uint32_t GetCacheValue(uint32_t address){

}
void SetCacheValue(uint32_t address, uint32_t value){

}

void HandleLoadCache(){

}
void HandleStoreCache(){

}

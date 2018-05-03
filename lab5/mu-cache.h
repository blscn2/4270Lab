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
	

	L1Cache.blocks[index].words[0] = mem_read_32(base_addr);
	L1Cache.blocks[index].words[1] = mem_read_32(base_addr + 4);
	L1Cache.blocks[index].words[2] = mem_read_32(base_addr + 8);
	L1Cache.blocks[index].words[3] = mem_read_32(base_addr + 12);
	
	
	L1Cache.blocks[index].valid = 1;
	L1Cache.blocks[index].tag = tag;

}

uint8_t CheckInCache(uint32_t address){
	uint8_t index = GETINDEX(address);
    uint32_t tag = GETTAG(address);
    
    //check if block is in cache at correct index and currently valid
    if( L1Cache.blocks[index].tag == tag && L1Cache.blocks[index].valid == 1){
		printf("\nCache hit at %08x",address);
		cache_hits++;
        return 1;
	}
    else{
		printf("\nCache miss at %08x",address);
		cache_misses++;
        return 0;
	}
}
uint32_t GetCacheValue(uint32_t address){
	return L1Cache.blocks[GETINDEX(address)].words[GETBLOCKOFF(address)];
}
void SetCacheValue(uint32_t address, uint32_t value){
	L1Cache.blocks[GETINDEX(address)].words[GETBLOCKOFF(address)] = value;
}
void HandleLoadCache(){
		if(ENABLE_CACHE == 1) {
		if(MEM_STALL <= 0){
			uint8_t bHitOrMiss = CheckInCache(EX_MEM.ALUOutput);
			uint32_t WriteBuffer;
			if(bHitOrMiss){
				MEM_STALL = 0;
				WriteBuffer = GetCacheValue(EX_MEM.ALUOutput);
				switch(MEM_WB.num_bytes){					
					case BYTE: 		MEM_WB.LMD = 0x00FF & WriteBuffer;	break;
					case HALF_WORD: MEM_WB.LMD = 0xFFFF & WriteBuffer;	break;
					case WORD:		MEM_WB.LMD = WriteBuffer;			break;
					default: 		/*	Do nothing	*/										break;
				}
			}
			else{
				LoadCache(EX_MEM.ALUOutput);
				MEM_STALL = 100;
			}
		}
		else
			MEM_STALL--;
	}
	else{
		if(MEM_STALL == 1){
			uint32_t WriteBuffer = mem_read_32( EX_MEM.ALUOutput );
			switch(MEM_WB.num_bytes){					
				case BYTE: 		MEM_WB.LMD = 0x00FF & WriteBuffer;	break;
				case HALF_WORD: MEM_WB.LMD = 0xFFFF & WriteBuffer;	break;
				case WORD:		MEM_WB.LMD = WriteBuffer;			break;
				default: 		/*	Do nothing	*/										break;
			}
			MEM_STALL = 0;
		}
		else if(MEM_STALL == 0)
			MEM_STALL = 100;
		else
			MEM_STALL--;
	}
}	
void HandleStoreCache(){
	if(ENABLE_CACHE == 1) {
		if(MEM_STALL <= 0){
			uint8_t bHitOrMiss = CheckInCache(EX_MEM.ALUOutput);
			if(bHitOrMiss){
				MEM_STALL = 0;
				switch(EX_MEM.num_bytes){
					case BYTE:	
						SetCacheValue(EX_MEM.ALUOutput, 0x00FF & EX_MEM.B);	
						mem_write_32(EX_MEM.ALUOutput, 0x00FF & EX_MEM.B); 		
						break;
					case HALF_WORD: 
						SetCacheValue(EX_MEM.ALUOutput, 0xFFFF & EX_MEM.B);
						mem_write_32(EX_MEM.ALUOutput, 0xFFFF & EX_MEM.B); 													
						break;			
					case WORD: 		
						SetCacheValue(EX_MEM.ALUOutput, EX_MEM.B);						
						mem_write_32(EX_MEM.ALUOutput, EX_MEM.B); 				
						break;
					default: 		/*	Do nothing	*/										
						break;				
				}
			}
			else{
				LoadCache(EX_MEM.ALUOutput);
				MEM_STALL = 100;
			}
		}
		else {
			if(MEM_STALL == 1){
				SetCacheValue(EX_MEM.ALUOutput, EX_MEM.B);						
				mem_write_32(EX_MEM.ALUOutput, EX_MEM.B); 
			}
				
			MEM_STALL--;
		}
	}
	else{
		if(MEM_STALL == 1){
			switch(MEM_WB.num_bytes){					
				case BYTE: 		mem_write_32(EX_MEM.ALUOutput, 0x00FF & EX_MEM.B); 	break;
				case HALF_WORD: mem_write_32(EX_MEM.ALUOutput, 0xFFFF & EX_MEM.B);	break;
				case WORD:		mem_write_32(EX_MEM.ALUOutput, EX_MEM.B);			break;
				default: 		/*	Do nothing	*/									break;
			}
			MEM_STALL = 0;
		}
		else if(MEM_STALL == 0)
			MEM_STALL = 100;
		else
			MEM_STALL--;
	}	
}	
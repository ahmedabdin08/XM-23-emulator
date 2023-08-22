#pragma once
/* Header for the Cache and its functions
/* Author: Ahmed Khairallah
*/
#include "memory.h"

#define CACHESIZE 16
#define DIRECTMAPPINGMASK 0x1F
#define DIRECTMAPPINGHASH(x) (x & DIRECTMAPPINGMASK) >> 1;
#define HYBRIDMASK 0x18 
#define HYBRIDCHUNCKHASH(x) (x & HYBRIDMASK) >> 3
#define CHUNKSIZE 4



#pragma warning(disable: 4996)

#define CACHESIZE 16 //this changes the hashing algorithms

typedef struct cache_line {
	unsigned short address;//address must be an even byte address
	word_byte data;
	unsigned short age : 4;
	unsigned short used : 1;
	unsigned short dirty : 1;
}cache_line;

enum{ASSOCIATIVE, DIRECT, HYBRID};//linear, hashing, chunks 
enum{WRITETHROUGH, WRITEBACK};//write through = always write writeback = write when a miss

extern unsigned short cache_flag, write_flag;//initially set to hybrid and writeback coder could change them though if wanted
extern float hitRatio;
extern cache_line cache[CACHESIZE];
enum { MISS, HIT };

void decrementAge(unsigned short age, unsigned short hybrid_index);
int leastRecentlyUsed(unsigned short hybrid_index);//returns cache index to be removed
void inspectCache();

void Cache(unsigned short mar, unsigned short* mbr, ReadOrWrite rw, Size wb);
void writing(unsigned short cache_index);
void read_write_cache(unsigned short* mbr, ReadOrWrite rw, Size wb, unsigned short index);
void updateRatio(unsigned short status);

int updateCacheHybrid(unsigned short address, unsigned short* mbr, ReadOrWrite rw, Size wb, unsigned short mar);
int updateCacheDirect(unsigned short cache_line_no, unsigned short* mbr, ReadOrWrite rw, Size wb, unsigned short mar);
int updateCacheAssociative(unsigned short address, unsigned short* mbr, ReadOrWrite rw, Size wb);
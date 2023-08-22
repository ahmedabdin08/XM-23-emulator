#pragma once
#include "cpu.h"
#include "cache.h"

#define MRU 15
#define MRUHYB 3

cache_line cache[CACHESIZE];

enum { NOTUSED, USED };
enum{FALSE, TRUE};
enum{LOWBYTE, HIGHBYTE};
unsigned short byteFlag  = LOWBYTE;
unsigned short totalRatio = 0, totalHit = 0;
float hitRatio = 0;
unsigned short cache_flag = HYBRID, write_flag = WRITEBACK;

void Cache(unsigned short mar, unsigned short* mbr, ReadOrWrite rw, Size wb) {
	unsigned short cache_line_no = 0;
	byteFlag = mar % 2;//should return HIGHBYTE if odd address and LOWBYTE if even address
	cache_line_no = mar & EVENMASK;//must be even byte address
	if(cache_flag == ASSOCIATIVE) updateCacheAssociative(cache_line_no, mbr, rw, wb) == HIT ? updateRatio(HIT) : updateRatio(MISS);
	if(cache_flag == DIRECT) updateCacheDirect(cache_line_no, mbr, rw, wb, mar) == HIT ? updateRatio(HIT) : updateRatio(MISS);
	if(cache_flag == HYBRID) updateCacheHybrid(cache_line_no, mbr, rw, wb, mar) == HIT ? updateRatio(HIT) : updateRatio(MISS);
	CPU_clock++;
}

void writing(unsigned short index) {
	if (write_flag == WRITETHROUGH)//we write back to memory
		bus(cache[index].address, &cache[index].data.word, WRITE, WORD);
	else //we set the dirty bit to write when a miss occurs
		cache[index].dirty = 1; 
}

void updateRatio(unsigned short status) {
	if (status == HIT) totalHit++;
	totalRatio++;
	hitRatio = ((float)totalHit / (float)totalRatio) * 100;
}

void read_write_cache(unsigned short* mbr, ReadOrWrite rw, Size wb, unsigned short index) {
	word_byte result;
	result.word = *mbr;
	switch (rw) {
	case READ:
		if (wb == WORD) result.word = cache[index].data.word;
		else result.byte[byteFlag] = cache[index].data.byte[byteFlag];
		break;
	case WRITE:
		if (wb == WORD) cache[index].data.word = result.word;
		else cache[index].data.byte[byteFlag] = result.byte[byteFlag];
		writing(index);
		break;
	}
	*mbr = result.word;
}

int updateCacheAssociative(unsigned short address, unsigned short *mbr, ReadOrWrite rw, Size wb) {
	int index = 0;
	for (int i = 0; i < CACHESIZE; i++) {
		if (cache[i].used == NOTUSED) {
			cache[i].used = USED;
			bus(address, &cache[i].data.word, READ, WORD);
			cache[i].address = address;
			decrementAge(cache[i].age, 0);
			cache[i].age = MRU;
			cache[i].dirty = 0;
			read_write_cache(mbr, rw, wb, i);
			return MISS;
		}
		else if (cache[i].address == address) {
			decrementAge(cache[i].age, 0);
			cache[i].age = MRU;
			read_write_cache(mbr, rw, wb, i);
			return HIT;
		}
	}

	index = leastRecentlyUsed(0);
	if (cache[index].dirty) bus(cache[index].address, &cache[index].data.word, WRITE, WORD);//WRITETHROUGH is assumed
	cache[index].address = address;
	bus(address, &cache[index].data.word, READ, WORD);
	cache[index].dirty = 0;
	read_write_cache(mbr, rw, wb, index);
	decrementAge(cache[index].age, 0);
	cache[index].age = MRU;
	return MISS;
}

int updateCacheHybrid(unsigned short address, unsigned short* mbr, ReadOrWrite rw, Size wb, unsigned short mar) {
	unsigned short hybrid_index = 0, index = 0;
	hybrid_index = HYBRIDCHUNCKHASH(mar);
	for (int i = hybrid_index * CHUNKSIZE; i < hybrid_index * CHUNKSIZE + CHUNKSIZE; i++) {
		if (cache[i].used == NOTUSED) {
			cache[i].used = USED;
			bus(address, &cache[i].data.word, READ, WORD);
			cache[i].address = address;
			decrementAge(cache[i].age, hybrid_index);
			cache[i].age = MRUHYB;
			cache[i].dirty = 0;
			read_write_cache(mbr, rw, wb, i);
			return MISS;
		}
		else if (cache[i].address == address) {
			decrementAge(cache[i].age, hybrid_index);
			cache[i].age = MRUHYB;
			read_write_cache(mbr, rw, wb, i);
			return HIT;
		}
	}

	index = leastRecentlyUsed(hybrid_index);
	if (cache[index].dirty) bus(cache[index].address, &cache[index].data.word, WRITE, WORD);//WRITETHROUGH is assumed
	cache[index].address = address;
	bus(address, &cache[index].data.word, READ, WORD);
	cache[index].dirty = 0;
	read_write_cache(mbr, rw, wb, index);
	decrementAge(cache[index].age, hybrid_index);
	cache[index].age = MRUHYB;
	return MISS;
}

int updateCacheDirect(unsigned short cache_line_no, unsigned short* mbr, ReadOrWrite rw, Size wb, unsigned short mar) {
	unsigned short direct_index = 0, status = MISS;
	direct_index = DIRECTMAPPINGHASH(mar);
	if (cache[direct_index].used == NOTUSED) {
		cache[direct_index].used = USED;
		bus(cache_line_no, &cache[direct_index].data.word, READ, WORD);
		cache[direct_index].dirty = 0;
		cache[direct_index].address = cache_line_no;
	}
	else if (cache[direct_index].address != cache_line_no) {
		if (cache[direct_index].dirty) bus(cache[direct_index].address, &cache[direct_index].data.word, WRITE, WORD);//write to memory
		bus(cache_line_no, &cache[direct_index].data.word, READ, WORD);
		cache[direct_index].address = cache_line_no;
		cache[direct_index].dirty = 0;
	}
	else status = HIT;
	read_write_cache(mbr, rw, wb, direct_index);
	return status;
}

int leastRecentlyUsed(unsigned short hybrid_index) {
	if (cache_flag == ASSOCIATIVE) {
		for (int i = 0; i < CACHESIZE; i++) {
			if (cache[i].age == 0) {
				return i;
			}
		}
	}
	else if (cache_flag == HYBRID) {
		for (int i = hybrid_index * CHUNKSIZE; i < hybrid_index * CHUNKSIZE + CHUNKSIZE; i++) {
			if (cache[i].age == 0) {
				return i;
			}
		}
	}
}

void decrementAge(unsigned short age, unsigned short hybrid_index) {
	if (cache_flag == ASSOCIATIVE) {
		for (int i = 0; i < CACHESIZE; i++) {
			if (cache[i].age > age) {
				cache[i].age--;
			}
		}
	}
	else if(cache_flag == HYBRID){
		for (int i = hybrid_index * CHUNKSIZE; i < hybrid_index * CHUNKSIZE + CHUNKSIZE; i++) {
			if (cache[i].age > age) {
				cache[i].age--;
			}
		}
	}
}

void inspectCache() {
	printf("Cache policy: ");
	switch (cache_flag)
	{
	case ASSOCIATIVE:
		printf("Associative");
		break;
	case DIRECT:
		printf("Direct");
		break;
	case HYBRID:
		printf("Hybrid");
		break;
	}

	printf("\nWrite Policy: ");
	write_flag == WRITETHROUGH ? printf("Write through") : printf("Write back");
	printf("\nHit Ratio: %.2f\n", hitRatio);
	
	printf("index\taddress\tdata\tage\tchanged\t\tusage\n");
	for (int i = 0; i < CACHESIZE; i++) {
		printf("%d\t%.4X\t%.4X\t%d\t", i, cache[i].address, cache[i].data.word, cache[i].age);
		cache[i].dirty ? printf("changed\t\t") : printf("not changed\t");
		cache[i].used ? printf("used\n") : printf("not used\n");
	}
}

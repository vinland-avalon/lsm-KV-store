#ifndef _BLOOM_FILTER_H_
#define _BLOOM_FILTER_H_

#include "mem_table.h"

class BloomFilter {
  public:
    BloomFilter(int bits_per_key);
    void CalculateFilterTable(MemTable immutable_memtbale, std::string* dst);
    bool KeyMayExist(const std::string& key, const std::string& filter);
};

#endif
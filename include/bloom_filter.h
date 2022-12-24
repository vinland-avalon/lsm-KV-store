#ifndef _BLOOM_FILTER_H_
#define _BLOOM_FILTER_H_

#include "mem_table.h"

// class BloomFilter {
//   public:
//     BloomFilter(int bits_per_key) : bits_per_key_(bits_per_key_) {
//         hash_times_ = bits_per_key_ * 0.69; // ln2
//     }
//     void CalculateFilterTable(MemTable &immutable_memtbale, int n, std::string *dst) {
//         size_t bits = n * bits_per_key_;
//         if (bits < 64)
//             bits = 64;
//         size_t bytes = (bits + 7) / 8;
//         bits = bytes * 8;
//         dst->resize(bytes, 0);
//     }

//     bool KeyMayExist(const std::string &key, const std::string &filter);

//   private:
//     int bits_per_key_;
//     int hash_times_;
// };

class BloomFilter {
  public:
    static void CalculateFilterTable(MemTable &immutable_memtbale, std::string *dst) {
        dst->resize(sizeof(size_t), 0);

        char *array = &(*dst)[0];
        int char_num = dst->size() / sizeof(char);

        size_t hash_table;
        for (immutable_memtbale.ReachBegin(); immutable_memtbale.Curr(); immutable_memtbale.Next()) {
            size_t hash_code = hash_algorithm_(immutable_memtbale.Curr()->GetKey());
            char *hash_code_byte = (char *)&hash_code;
            for (int i = 0; i < char_num; i++) {
                array[i] |= hash_code_byte[i];
            }
        }
    }

    static bool KeyMayExist(std::string &key, std::string &filter) {
        if (filter.size() != sizeof(size_t))
            return false;

        char *array = &(filter)[0];
        int char_num = filter.size() / sizeof(char);

        size_t hash_code = hash_algorithm_(key);
        char *hash_code_byte = (char *)&hash_code;

        for (int i = 0; i < char_num; i++) {
            if ((array[i] ^ hash_code_byte[i]) != hash_code_byte[i]) {
                return false;
            }
        }

        return true;
    }

  private:
    static std::hash<std::string> hash_algorithm_;
};

#endif
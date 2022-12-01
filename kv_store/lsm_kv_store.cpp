/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 10:30:28
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-01 01:40:18
 * @FilePath: /lsm-KV-store/kv_store/lsm_kv_store.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include "../command/command.cpp"
#include "../mem_table/mem_table.cpp"
#include "../mem_table/red_black_tree_impl.cpp"
#include "../sstable/ss_table.cpp"
#include "./kv_store.cpp"
#include <list>
#include <map>
#include <shared_mutex>

class LsmKvStore : public KvStore {
  public:
    LsmKvStore(std::string _dataDir, int _memThreshold, int _partitionSize) : dataDir(_dataDir), memThreshold(_memThreshold), partitionSize(_partitionSize) {
    }

    void set(std::string, std::string);
    std::string get(std::string);
    void remove(std::string);

  private:
    MemTable *memTable;
    MemTable *immutableMemTable;

    std::list<SsTable> ssTables;
    const std::string dataDir;
    const int memThreshold;
    const std::shared_mutex rwlock;
    const int partitionSize;

    void restoreFromWal();
    void switchMemTable();
    void flushToSsTable();

  protected:
};
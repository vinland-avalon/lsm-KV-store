/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 10:30:28
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-11-30 14:30:53
 * @FilePath: /lsm-KV-store/kv_store/lsm_kv_store.cpp
 * @Description: 
 * 
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved. 
 */
#include "./kv_store.cpp"
#include <map>
#include <list>
#include <shared_mutex>
#include "../command/command.cpp"
#include "../sstable/ss_table.cpp"
#include "../mem_table/red_black_tree_impl.cpp"
#include "../mem_table/mem_table.cpp"

class LsmKvStore: public KvStore {
public:
    LsmKvStore(std::string _dataDir, int _memThreshold, int _partitionSize):dataDir(_dataDir),memThreshold(_memThreshold),partitionSize(_partitionSize){
    }

    void set(std::string, std::string);
    std::string get(std::string);
    void remove(std::string);


private:
    MemTable* memTable;
    MemTable* immutableMemTable;
    
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
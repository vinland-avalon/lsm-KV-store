/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 10:30:28
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-03 23:33:25
 * @FilePath: /lsm-KV-store/kv_store/lsm_kv_store.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include "../command/command.h"
#include "../mem_table/mem_table.h"
#include "../mem_table/red_black_tree_impl.cpp"
#include "../sstable/ss_table.cpp"
#include "./kv_store.cpp"
#include "./utils/utils_for_file_operation.h"
#include <list>
#include <map>
#include <shared_mutex>

class LsmKvStore : public KvStore {
  public:
    const std::string TABLE_SUFFIX = ".table";
    const std::string WAL = "wal";
    const std::string WAL_TMP = "walTmp";
    // todo: only works for abolution filePath
    // todo: for sstable and kv_store, they have to see the files in the same path
    // todo: 1) both in ../ or 2) absulute path
    LsmKvStore(std::string _dataDir, int _memThreshold, int _partitionSize) : dataDir(_dataDir), memThreshold(_memThreshold), partitionSize(_partitionSize) {
        this->rwlock = new std::shared_mutex();
        this->ssTables = new std::list<SsTable *>();
        this->memTable = new RedBlackTreeMemTable();
        std::vector<std::string> *filesInDir = getFilenamesInDirectory(dataDir.c_str());
        if (filesInDir == nullptr) {
            return;
        }
        std::map<long, SsTable *> *ssTableMap = new std::map<long, SsTable *>();
        for (std::string filename : (*filesInDir)) {
            // restore from walTmp file, and such file commonly derive from ssTable that fails when being persisted
            if (filename == WAL_TMP) {
                restoreFromWal(filename);
            }
            if (endsWith(filename, TABLE_SUFFIX)) {
                long time = stol(filename.substr(filename.size() - TABLE_SUFFIX.size()));
                SsTable *ssTable = new SsTable(filename, 0);
                ssTable->initFromFile();
                ssTableMap->insert(std::make_pair(time, ssTable));
            } else if (filename == WAL) {
                walFile = filename;
                restoreFromWal(filename);
            }
        }
        for (auto it = ssTableMap->begin(); it != ssTableMap->end(); it++) {
            this->ssTables->push_back(it->second);
        }
    }

    void set(std::string, std::string);
    std::string get(std::string);
    void remove(std::string);

  private:
    MemTable *memTable;
    MemTable *immutableMemTable;

    std::list<SsTable *> *ssTables;
    const std::string dataDir;
    const int memThreshold;
    std::shared_mutex *rwlock;
    const int partitionSize;
    std::string walFile;

    void restoreFromWal(std::string filename);
    void switchMemTable();
    void flushToSsTable();

  protected:
};
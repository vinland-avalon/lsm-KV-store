/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 10:30:28
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-05 23:14:54
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
#include <cstdio>
#include <list>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <thread>
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
            if (endsWith(filename, TABLE_SUFFIX)) {
                long time = stol(filename.substr(filename.size() - TABLE_SUFFIX.size()));
                SsTable *ssTable = new SsTable(filename, 0);
                ssTable->initFromFile();
                ssTableMap->insert(std::make_pair(time, ssTable));

                // the latter is to restore from walTmp file, and such file commonly derive from ssTable that fails when being persisted
            } else if (filename == WAL || filename == WAL_TMP) {
                std::fstream *tmpFstream = new std::fstream;
                tmpFstream->open(walFile, std::ios::in | std::ios::out | std::ios::binary);
                if (!tmpFstream->is_open()) {
                    std::cout << filename << ": fail to open" << std::endl;
                }
                if (filename == WAL) {
                    this->walFile = filename;
                    this->walFileStream = tmpFstream;
                }
                restoreFromWal(tmpFstream);
            }
        }
        for (auto it = ssTableMap->begin(); it != ssTableMap->end(); it++) {
            this->ssTables->push_back(it->second);
        }
    }

    void set(std::string key, std::string value) {
        // todo: take the advantage of thread pool
        SetCommand *command = new SetCommand("SET", key, value);
        std::string commandString = command->toJSON().dump();
        std::unique_lock<std::shared_mutex> wlock(*(this->rwlock));

        // todo: write ahead log

        // memtable
        memTable->set(key, command);

        // if memtable is at threshold, make it consistent to SSD
        if (memTable->size() > memThreshold) {
            // memTable -> immutableMemTable
            switchMemTableAndWal();
            flushToSsTable();
        }
    }
    std::string get(std::string);
    void remove(std::string key) {
        // todo: take the advantage of thread pool
        RmCommand *command = new RmCommand("SET", key);
        std::string commandString = command->toJSON().dump();
        std::unique_lock<std::shared_mutex> wlock(*(this->rwlock));

        // todo: write ahead log

        // memtable
        memTable->set(key, command);

        // if memtable is at threshold, make it consistent to SSD
        if (memTable->size() > memThreshold) {
            // memTable -> immutableMemTable
            switchMemTableAndWal();
            flushToSsTable();
        }
    }

  private:
    MemTable *memTable;
    MemTable *immutableMemTable;

    std::list<SsTable *> *ssTables;
    const std::string dataDir;
    const int memThreshold;
    std::shared_mutex *rwlock;
    const int partitionSize;
    std::string walFile;
    std::fstream *walFileStream;

    void restoreFromWal(std::fstream *fileStream);
    void switchMemTableAndWal() {
        // switch MemTable
        this->immutableMemTable = this->memTable;
        this->memTable = new RedBlackTreeMemTable();
        // switch WAL file
        walFileStream->close();
        std::string fullWalFile = this->dataDir + WAL_TMP;
        if (isFileExisting(fullWalFile)) {
            if (!std::remove(fullWalFile.c_str())) {
                std::cout << "[switchMemTableAndWal] fail to delete walTmp: " << fullWalFile << std::endl;
            }
        }
        if (!std::rename(this->walFile.c_str(), fullWalFile.c_str())) {
            std::cout << "[switchMemTableAndWal] fail to rename walTmp: " << this->walFile << " to " << fullWalFile << std::endl;
        }
        this->walFile = fullWalFile;
        this->walFileStream->open(this->walFile, std::ios::in | std::ios::out | std::ios::binary);
    }
    void flushToSsTable();

  protected:
};
/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 10:30:28
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-06 18:52:50
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
#include "./utils/utils_for_time_operation.h"
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
        long len = commandString.size();
        std::unique_lock<std::shared_mutex> wlock(*(this->rwlock));

        // write ahead log
        this->walFileStream->write((const char *)&len, sizeof(len));
        writeStringToFile(commandString, this->walFileStream);

        // memtable
        memTable->set(key, command);

        // if memtable is at threshold, make it consistent to SSD
        if (memTable->size() > memThreshold) {
            // memTable -> immutableMemTable
            switchMemTableAndWal();
            flushToSsTable();
        }
    }
    std::string get(std::string key) {
        std::shared_lock<std::shared_mutex> rlock(*(this->rwlock));
        Command *command = this->memTable->get(key);
        if (command == nullptr && this->immutableMemTable != nullptr) {
            command = this->immutableMemTable->get(key);
        }
        if (command == nullptr) {
            for (SsTable *ssTable : *(this->ssTables)) {
                command = ssTable->query(key);
                if (command) {
                    break;
                }
            }
        }
        if (command == nullptr)
            return nullptr;
        if (command->getType() == "SET") {
            return (dynamic_cast<SetCommand *>(command))->getValue();
        }
        if (command->getType() == "RM") {
            return nullptr;
        }
    }
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

    /**
     * @description: wal/tmp_file data structure: len json len json
     * @param {fstream} *fileStream
     * @return {*}
     */
    void restoreFromWal(std::fstream *fileStream) {
        fileStream->seekg(0);
        long len = 0;
        char buffer[1000];
        while (fileStream->eof()) {
            fileStream->read((char *)&len, sizeof(len));
            fileStream->read(buffer, len);
            std::string commandString = buffer;
            Command *command = SsTable::JSONtoCommand(json::parse(commandString));
            this->memTable->set(command->getKey(), command);
        }
    }
    /**
     * @description: 1. memTable->immutableMemTable
     * @discription: 2. old walfile rename to WAL_TMP, create new walFile
     * @return {*}
     */
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
    /**
     * @description: 1. flush immutableMemTable to brand-new file, created in the form of dataDir + system time + table_suffix
     * @discription: 2. add the ssTable structure to list
     * @discription: 3. delete old immutableMemTable and WAL_TMP
     * @return {*}
     */
    void flushToSsTable() {
        std::string ssTableName = this->dataDir + getSystemTimeInMills() + this->TABLE_SUFFIX;
        SsTable *ssTable = new SsTable(ssTableName, this->partitionSize);
        ssTable->initFromMemTable(this->immutableMemTable);

        delete this->immutableMemTable;
        this->immutableMemTable = nullptr;
        std::string oldTmpWal = this->dataDir + WAL_TMP;
        if (isFileExisting(oldTmpWal)) {
            if (!std::remove(oldTmpWal.c_str())) {
                std::cout << "[flushToSsTable] fail to delete oldTmpWal: " << oldTmpWal << std::endl;
            }
        }
    }

  protected:
};
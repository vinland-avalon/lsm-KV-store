/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 10:30:28
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-11 11:51:40
 * @FilePath: /lsm-KV-store/include/lsm_kv_store.h
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#ifndef _LSM_KV_STORE_H_
#define _LSM_KV_STORE_H_

#include "command.h"
#include "kv_store.h"
#include "mem_table.h"
#include "mem_table_red_black_tree_impl.h"
#include "ss_table.h"
#include "utils_for_file_operation.h"
#include "utils_for_json_operation.h"
#include "utils_for_time_operation.h"
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
        try {
            this->rwlock = new std::shared_mutex();
            this->ssTables = new std::list<std::shared_ptr<SsTable>>();
            this->memTable = std::shared_ptr<MemTable>(new RedBlackTreeMemTable());
            if (!isSolidDirectory(this->dataDir)) {
                spdlog::warn("[LsmKvStore][constructor] dataDir: {} is invalid", this->dataDir);
                // std::string will cast to std::exception
                throw dataDir + " not valid";
            }
            std::vector<std::string> filesInDir = getFilenamesInDirectory(dataDir);
            // make it order: from big to small
            std::map<long, std::shared_ptr<SsTable>, std::greater<long>> ssTableMap;
            for (std::string filename : filesInDir) {
                std::string fileWithPath = dataDir + "/" + filename;
                // it's a SSTable file, it's like 142153253253.table
                if (endsWith(filename, TABLE_SUFFIX)) {
                    spdlog::info("[LsmKvStore][constructor] init ssTable from file: {}", filename);
                    long time = getTimeFromSSTableName(filename);
                    std::shared_ptr<SsTable> ssTable = SsTable::initFromSSD(fileWithPath);
                    ssTableMap.insert(std::make_pair(time, ssTable));
                    // WAL_TMP: restore from walTmp file, and such file commonly derive from ssTable that fails when being persisted
                    // WAL: restore from a pre-exist file, can keep on using it
                } else if (filename == WAL || filename == WAL_TMP) {
                    spdlog::info("[LsmKvStore][constructor] restore commands from file: {}", filename);
                    std::fstream *tmpFstream = new std::fstream;
                    openFileAndCreateOneWhenNotExist(tmpFstream, fileWithPath);
                    // tmpFstream->open(fileWithPath, std::ios::in | std::ios::out | std::ios::binary);
                    if (!tmpFstream->is_open()) {
                        std::cout << filename << ": fail to open" << std::endl;
                    }
                    if (filename == WAL) {
                        this->walFile = fileWithPath;
                        this->walFileStream = tmpFstream;
                    }
                    restoreFromWal(tmpFstream);
                }
            }
            if (this->walFile == "" || this->walFileStream == nullptr) {
                this->walFile = dataDir + "/" + WAL;
                this->walFileStream = new std::fstream;
                openFileAndCreateOneWhenNotExist(this->walFileStream, this->walFile);
            }
            for (auto it = ssTableMap.begin(); it != ssTableMap.end(); it++) {
                this->ssTables->push_back(it->second);
            }
        } catch (std::exception &error) {
            spdlog::error("[LsmKvStore][constructor] error: {}", error.what());
            throw error;
        }
    }

    void Set(std::string key, std::string value) {
        try {
            auto command = std::shared_ptr<SetCommand>(new SetCommand("SET", key, value));
            executeCommand(command);
        } catch (std::exception &error) {
            // the toppest function, so won't throw again
            spdlog::error("[LsmKvStore][Set] error: {}", error.what());
        }
    }

    void Remove(std::string key) {
        try {
            auto command = std::shared_ptr<RmCommand>(new RmCommand("RM", key));
            executeCommand(command);
        } catch (std::exception &error) {
            // the toppest function, so won't throw again
            spdlog::error("[LsmKvStore][Rm] error: {}", error.what());
        }
    }

    std::string Get(std::string key) {
        std::shared_lock<std::shared_mutex> rlock(*(this->rwlock));
        // try to get corresponding command in memTable
        std::shared_ptr<Command> command = this->memTable->get(key);
        // try to get corresponding command in immutableMemTable
        if (command == nullptr && this->immutableMemTable != nullptr) {
            command = this->immutableMemTable->get(key);
        }
        // try to get corresponding command in ssTables
        if (command == nullptr) {
            for (std::shared_ptr<SsTable> ssTable : *(this->ssTables)) {
                command = ssTable->query(key);
                if (command) {
                    break;
                }
            }
        }
        // check the "get" result and return value;
        if (command == nullptr)
            return "";
        else if (command->getType() == "SET") {
            std::string value = (std::dynamic_pointer_cast<SetCommand>(command))->getValue();
            spdlog::info("[LsmKvStore][Get] get {} for key: {}", value, key);
            return value;
        } else if (command->getType() == "RM") {
            return "";
        }
        return "";
    }

    ~LsmKvStore() {
        flushSSTableAndWal();
        delete this->walFileStream;
        delete this->rwlock;
        delete this->ssTables;
    }

  private:
    std::shared_ptr<MemTable> memTable;
    std::shared_ptr<MemTable> immutableMemTable;

    // std::list<SsTable *> *ssTables;
    std::list<std::shared_ptr<SsTable>> *ssTables;
    const std::string dataDir;
    const int memThreshold;
    std::shared_mutex *rwlock;
    const int partitionSize;
    std::string walFile = "";
    std::fstream *walFileStream = nullptr;

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
            auto command = JSONtoCommand(json::parse(commandString));
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
        this->memTable = std::shared_ptr<RedBlackTreeMemTable>(new RedBlackTreeMemTable());
        // switch WAL file
        walFileStream->close();
        std::string fullWalFile = this->dataDir + "/" + WAL_TMP;
        if (isFileExisting(fullWalFile)) {
            if (!std::remove(fullWalFile.c_str())) {
                spdlog::error("[LsmKvStore][switchMemTableAndWal] fail to delete walTmp: {}", fullWalFile);
                throw "fail to delete WAL_TMP";
            }
        }
        if (std::rename(this->walFile.c_str(), fullWalFile.c_str())) {
            spdlog::error("[LsmKvStore][switchMemTableAndWal] fail to rename walTmp: {} to {}", this->walFile, fullWalFile);
            throw "fail to rename WAL to WAL_TMP";
        }
        this->walFile = fullWalFile;
        openFileAndCreateOneWhenNotExist(this->walFileStream, this->walFile);
        // this->walFileStream->open(this->walFile, std::ios::in | std::ios::out | std::ios::binary);
        spdlog::info("[LsmKvStore][switchMemTableAndWal] rename walTmp: {} to {}", this->walFile, fullWalFile);
    }
    /**
     * @description: 1. flush immutableMemTable to brand-new file, created in the form of dataDir + system time + table_suffix
     * @discription: 2. add the ssTable structure to list
     * @discription: 3. delete old immutableMemTable and WAL_TMP
     * @return {*}
     */
    void flushToSSD() {
        std::string ssTablePath = this->dataDir + "/" + getSystemTimeInMills() + this->TABLE_SUFFIX;
        std::shared_ptr<SsTable> ssTable = SsTable::initFromMemTableAndFlushToSSD(this->immutableMemTable, this->partitionSize, ssTablePath);

        this->immutableMemTable = nullptr;
        std::string oldTmpWal = this->dataDir + "/" + WAL_TMP;
        if (isFileExisting(oldTmpWal)) {
            if (std::remove(oldTmpWal.c_str())) {
                spdlog::error("[LsmKvStore][flushToSsTable] fail to delete oldTmpWal: {}", oldTmpWal);
            }
        }
    }

    void executeCommand(std::shared_ptr<Command> command) {
        try {
            // todo: take the advantage of thread pool
            std::string commandString = command->toJSON().dump();
            long len = commandString.size();
            std::unique_lock<std::shared_mutex> wlock(*(this->rwlock));

            // write ahead log
            this->walFileStream->write((const char *)&len, sizeof(len));
            writeStringToFile(commandString, this->walFileStream);

            // memtable
            memTable->set(command->getKey(), command);

            // if memtable is at threshold, make it consistent to SSD
            if (memTable->size() > memThreshold) {
                flushSSTableAndWal();
            }
        } catch (std::exception &error) {
            spdlog::error("[LsmKvStore][executeCommand] error: {}", error.what());
            throw;
        }
    }

    void flushSSTableAndWal() {
        try {
            // memTable -> immutableMemTable
            // new memTable
            switchMemTableAndWal();
            // todo: make the flush action to async
            flushToSSD();
        } catch (std::exception &error) {
            spdlog::error("[LsmKvStore][flushSSTableAndWal] error: {}", error.what());
            throw;
        }
    }
    
    long getTimeFromSSTableName(std::string filename){
        int start = 0;
        // drop .table
        int len = filename.size() - TABLE_SUFFIX.size() - 1;
        long time = stol(filename.substr(start, len));
        return time;
    }

  protected:
};

#endif
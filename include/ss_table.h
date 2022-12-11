/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:33:21
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-10 19:45:38
 * @FilePath: /lsm-KV-store/include/ss_table.h
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#ifndef _SS_TABLE_H_
#define _SS_TABLE_H_

#include "command.h"
#include "mem_table.h"
#include "ss_table_meta_info.h"
#include "utils_for_file_operation.h"
#include "utils_for_json_operation.h"
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>
using json = nlohmann::json;
// data + sparse index + metainfo

class SsTable {
  public:
    ~SsTable() {
        tableFile.close();
    }
    // todo: single instance mode
    std::shared_ptr<TableMetaInfo> getTableMetaInfo() { return this->tableMetaInfo; }

    static std::shared_ptr<SsTable> initFromMemTableAndFlushToSSD(std::shared_ptr<MemTable> memTable, int partitionSize, std::string _filePath) {
        try{
            if(memTable == nullptr) {
                spdlog::error("[SsTable][initFromMemTableAndFlushToSSD] *memTable is nullptr");
                throw "*memTable is nullptr";
            }
            std::shared_ptr<SsTable> ssTable = std::shared_ptr<SsTable>(new SsTable(_filePath, partitionSize));
            ssTable->initFromMemTable(memTable);
            return ssTable;
        } catch (...) {
            throw;
        }
    }

    static std::shared_ptr<SsTable> initFromSSD(std::string _filePath) {
        try{
            std::shared_ptr<SsTable> ssTable = std::shared_ptr<SsTable>(new SsTable(_filePath));
            ssTable->reloadWithGivenSSDFile(_filePath);
            return ssTable;
        } catch (...) {
            throw;
        }
    }

    /**
     * @description: get the most recently commmand of a key in this sstable;
     * @discription: we will look up sparse index first, and then get target command in a specific block;
     * @discriotion: it's lazy loading and thus reduce io;
     * @param {string} key
     * @return {*}
     */
    std::shared_ptr<Command> query(std::string key) {
        // start, len
        std::pair<long, long> lowerBoundBlock(-1, -1);
        for (auto it = sparseIndex->begin(); it != sparseIndex->end(); it++) {
            if (it->first <= key) {
                lowerBoundBlock.first = it->second.first;
                lowerBoundBlock.second = it->second.second;
            }
        }
        if (lowerBoundBlock.first == -1){
            spdlog::info("[SsTable][query] not even found right block in this block");
            return nullptr;
        }
            

        // load sparse index
        char buffer[1000];
        tableFile.seekg(lowerBoundBlock.first);
        tableFile.read(buffer, lowerBoundBlock.second);
        // std::string blockString = buffer;
        // {"key1":{"key":"key1","type":"SET","value":"100"}}{"key2":{"key":"key2","type":"SET","value":"101"}}
        json blockJSON = json::parse(buffer);
        spdlog::info("[SsTable][query] the needed command maybe in this block, data:{}", blockJSON.dump());

        for (json::iterator it = blockJSON.begin(); it != blockJSON.end(); ++it) {
            //std::cout << "[query] reload command in block: " << it.key() << " : " << it.value() << "\n";
            if (it.key() == key) {
                spdlog::info("[SsTable][query] get the needed command:{}", it.value().dump());
                return JSONtoCommand(it.value());
            }
        }
        return nullptr;
    }
    
    // void writecommands(json commands) {}

  private:
    std::shared_ptr<TableMetaInfo> tableMetaInfo;
    std::fstream tableFile;
    // sparseIndex:{ key: {start, len}}
    std::shared_ptr<std::map<std::string, std::pair<long, long>>> sparseIndex;
    std::string filePath;
    SsTable(std::string _filePath, int _partitionSize = 0) {
        try {
            this->tableMetaInfo = std::shared_ptr<TableMetaInfo>(new TableMetaInfo());
            this->tableMetaInfo->setPartitionSize(_partitionSize);
            this->filePath = _filePath;
            this->sparseIndex = std::shared_ptr<std::map<std::string, std::pair<long, long>>>(new std::map<std::string, std::pair<long, long>>());
            openFileAndCreateOneWhenNotExist(&(this->tableFile), this->filePath);
            //tableFile.open(_filePath, std::ios::in | std::ios::out | std::ios::binary);
            if (!tableFile.is_open()) {
                spdlog::error("[SsTable][Constructor] fail to open file {}", this->filePath);
                throw "Fail to open file";
            }
            tableFile.seekp(0);
            tableFile.seekg(0);
            spdlog::info("[SsTable][Constructor] initialized an instance, filePath: {}, metainfo.partitionSize: {}", this->filePath, this->tableMetaInfo->getPartitionSize());
        } catch (std::string error) {
            throw error;
        } catch (std::exception &error) {
            tableFile.close();
            throw error;
        }
    }
    
    /**
     * @description: init from memtable and flush memtable to sstable
     * @param {string} _filePath
     * @param {int} _partitionSize
     * @param {MemTable*} memtable
     * @return {*} the pointer to SsTable
     */
    void initFromMemTable(std::shared_ptr<MemTable> memtable) {
        // calculate metadata
        tableMetaInfo->setDataStart(tableFile.tellp());

        // flush to SSD, part by part
        json commands;
        memtable->reachBegin();
        while (memtable->curr()) {
            commands[memtable->curr()->getKey()] = memtable->curr()->toJSON();
            std::cout << " get record: " << commands << std::endl;
            spdlog::info("[SsTable][initFromMemTable] get command {} from immutableMemTable", memtable->curr()->toJSON().dump());
            memtable->next();
            if (commands.size() >= tableMetaInfo->getPartitionSize()) {
                writeToSSDandClearAndAppendSparseIndex(&commands);
            }
        }
        if (commands.size() > 0) {
            writeToSSDandClearAndAppendSparseIndex(&commands);
        }


        // calculate metadata
        long dataPartLen = tableFile.tellp() - tableMetaInfo->getDataStart();
        tableMetaInfo->setDataLen(dataPartLen);

        // save sparse index and calculate metadata
        tableMetaInfo->setIndexStart(tableFile.tellp());
        json sparseIndexJSON;
        for (auto singleSparkIndex : (*sparseIndex)) {
            // key:{start, len}
            sparseIndexJSON[singleSparkIndex.first] = (json{singleSparkIndex.second.first, singleSparkIndex.second.second});
        }
        std::string sparseIndexString = sparseIndexJSON.dump();
        spdlog::info("[SsTable][initFromMemTable] sparse index: {}", sparseIndexString);
        writeStringToFile(sparseIndexString, &tableFile);
        // tableFile.write(sparseIndexString.c_str(), sparseIndexString.size() + 1);
        // tableFile << sparseIndex;
        tableMetaInfo->setIndexLen(tableFile.tellp() - tableMetaInfo->getIndexStart());

        // save metadata
        tableMetaInfo->writeToFile(&tableFile);

        tableFile.flush();
    }

    /**
     * @description: from SSD, restore ssTable to memory
     * @description: only load metadata and sparse index
     * @param {string} _filePath
     * @return {*}
     */
    void reloadWithGivenSSDFile(std::string _filePath) {
        // load metadata
        this->tableMetaInfo = std::shared_ptr<TableMetaInfo>(new TableMetaInfo());
        tableMetaInfo->readFromFile(&(this->tableFile));

        // load sparse index
        char buffer[1000];
        tableFile.seekg(tableMetaInfo->getIndexStart());
        tableFile.read(buffer, tableMetaInfo->getIndexLen());

        // std::string sparseIndexString = buffer;

        // {"key1":[0,50]}
        // // std::cout << "lll" << sparseIndexString << std::endl;

        // json tmpJSONcommands = json::parse(sparseIndexString);
        json tmpJSONcommands = json::parse(buffer);
        spdlog::info("[SsTable][initFromFile] reload sparse index {}", tmpJSONcommands.dump());

        for (json::iterator it = tmpJSONcommands.begin(); it != tmpJSONcommands.end(); ++it) {
            // std::cout << "[initFromFile] reload sparse index: " << it.key() << " : " << it.value() << "\n";
            sparseIndex->emplace(it.key(), std::pair<long, long>(it.value().at(0), it.value().at(1)));
        }
    }

    /**
     * @description: write commands to SSD, clear commands in JSON, and then append sparse index entry to this instance
     * @param {json} *commands
     * @return {*}
     */
    void writeToSSDandClearAndAppendSparseIndex(json *commands) {
        spdlog::info("[SsTable][writeToSSDandClearAndAppendSparseIndex] write commands: {}", commands->dump());
        std::string key = (*(commands->begin()))["key"];
        long start = tableFile.tellp();

        writeStringToFile(commands->dump(), &tableFile);
        
        commands->clear();
        
        long len = tableFile.tellp() - start;
        sparseIndex->emplace(key, std::pair<long, long>(start, len));
    }
};

#endif
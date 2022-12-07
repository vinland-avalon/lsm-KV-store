/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:33:21
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-07 17:28:33
 * @FilePath: /lsm-KV-store/src/ss_table.h
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include "mem_table.h"
#include "utils_for_file_operation.h"
#include "ss_table_meta_info.h"
#include "command.h"
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;
// data + sparse index + metainfo

class SsTable {
  public:
    SsTable(std::string _filePath, int _partitionSize) {
        this->tableMetaInfo = new TableMetaInfo();
        this->tableMetaInfo->setPartitionSize(_partitionSize);
        this->filePath = _filePath;
        this->sparseIndex = new std::multimap<std::string, std::pair<long, long>>();
        tableFile.open(_filePath, std::ios::in | std::ios::out | std::ios::binary);
        if (!tableFile.is_open()) {
            std::cout << filePath << ": fail to open" << std::endl;
        }
        tableFile.seekp(0);
        tableFile.seekg(0);
    }
    ~SsTable() {
        tableFile.close();
    }
    // todo: single instance mode
    TableMetaInfo *getTableMetaInfo() { return this->tableMetaInfo; }

    static Command *JSONtoCommand(json jCommand) {
        Command *command = nullptr;
        if (jCommand["type"] == "SET") {
            command = new SetCommand("SET", jCommand["key"], jCommand["value"]);
        } else if (jCommand["type"] == "RM") {
            command = new RmCommand("RM", jCommand["key"]);
        }
        return command;
    };

    /**
     * @description: init from memtable and flush memtable to sstable
     * @param {string} _filePath
     * @param {int} _partitionSize
     * @param {MemTable*} memtable
     * @return {*} the pointer to SsTable
     */
    SsTable *initFromMemTable(MemTable *memtable) {
        // calculate metadata
        tableMetaInfo->setDataStart(tableFile.tellp());

        // flush to SSD
        json records = json({});
        memtable->reachBegin();
        while (memtable->curr()) {
            records[memtable->curr()->getKey()] = memtable->curr()->toJSON();

            std::cout << "[initFromMemTable] get record: " << records << std::endl;
            memtable->next();
            if (records.size() >= tableMetaInfo->getPartitionSize()) {
                writeToSSDandClearAndAppendSparseIndex(&records);
            }
        }
        if (records.size() > 0) {
            writeToSSDandClearAndAppendSparseIndex(&records);
        }

        // calculate metadata
        long dataPartLen = tableFile.tellp() - tableMetaInfo->getDataStart();
        tableMetaInfo->setDataLen(dataPartLen);

        // save sparse index and calculate metadata
        tableMetaInfo->setIndexStart(tableFile.tellp());
        json sparseIndexJSON;
        for (auto singleSparkIndex : (*sparseIndex)) {
            sparseIndexJSON[singleSparkIndex.first] = (json{singleSparkIndex.second.first, singleSparkIndex.second.second});
        }
        std::string sparseIndexString = sparseIndexJSON.dump();
        writeStringToFile(sparseIndexString, &tableFile);
        // tableFile.write(sparseIndexString.c_str(), sparseIndexString.size() + 1);
        // tableFile << sparseIndex;
        tableMetaInfo->setIndexLen(tableFile.tellp() - tableMetaInfo->getIndexStart());

        // save metadata
        tableMetaInfo->writeToFile(&tableFile);

        tableFile.flush();

        return this;
    }
    /**
     * @description: from SSD, restore ssTable to memory
     * @description: only load metadata and sparse index
     * @param {string} _filePath
     * @return {*}
     */
    SsTable *initFromFile() {
        // load metadata
        this->tableMetaInfo = new TableMetaInfo();
        tableMetaInfo->readFromFile(&(this->tableFile));

        // load sparse index
        char buffer[1000];
        tableFile.seekg(tableMetaInfo->getIndexStart());
        tableFile.read(buffer, tableMetaInfo->getIndexLen());

        std::string sparseIndexString = buffer;

        // {"key1":[0,50]}
        // std::cout << "lll" << sparseIndexString << std::endl;

        json tmpJSONRecords = json::parse(sparseIndexString);

        for (json::iterator it = tmpJSONRecords.begin(); it != tmpJSONRecords.end(); ++it) {
            std::cout << "[initFromFile] reload sparse index: " << it.key() << " : " << it.value() << "\n";
            sparseIndex->emplace(it.key(), std::pair<long, long>(it.value().at(0), it.value().at(1)));
        }

        return this;
    }
    /**
     * @description: get the most recently commmand of a key in this sstable;
     * @discription: we will look up sparse index first, and then get target command in a specific block;
     * @discriotion: it's lazy loading and thus reduce io;
     * @param {string} key
     * @return {*}
     */
    Command *query(std::string key) {
        std::pair<long, long> lowerBoundBlock(-1, -1);
        for (auto it = sparseIndex->begin(); it != sparseIndex->end(); it++) {
            if (it->first <= key) {
                lowerBoundBlock.first = it->second.first;
                lowerBoundBlock.second = it->second.second;
            }
        }
        if (lowerBoundBlock.first == -1)
            return nullptr;

        // load sparse index
        char buffer[1000];
        tableFile.seekg(lowerBoundBlock.first);
        tableFile.read(buffer, lowerBoundBlock.second);
        std::string blockString = buffer;
        // {"key1":{"key":"key1","type":"SET","value":"100"}}{"key2":{"key":"key2","type":"SET","value":"101"}}
        json blockJSON = json::parse(blockString);

        for (json::iterator it = blockJSON.begin(); it != blockJSON.end(); ++it) {
            std::cout << "[query] reload command in block: " << it.key() << " : " << it.value() << "\n";
            if (it.key() == key) {
                return JSONtoCommand(it.value());
            }
        }
        return nullptr;
    }
    // void writeRecords(json records) {}
    /**
     * @description: write records to SSD, clear records in JSON, and then append sparse index entry to this instance
     * @param {json} *records
     * @return {*}
     */
    void writeToSSDandClearAndAppendSparseIndex(json *records) {
        std::cout << "[writeToSSDandClearAndAppendSparseIndex] write records:" << *records << "to SSD" << std::endl;
        std::string key = (*(records->begin()))["key"];
        long start = tableFile.tellp();
        std::string recordsString = records->dump();
        writeStringToFile(recordsString, &tableFile);
        // tableFile.write(recordsString.c_str(), recordsString.size() + 1);
        // tableFile << records->dump();
        records->clear();
        long len = tableFile.tellp() - start;
        sparseIndex->emplace(key, std::pair<long, long>(start, len));
    }

  private:
    TableMetaInfo *tableMetaInfo;
    std::fstream tableFile;
    // sparseIndex:{ key: {start, len}}
    std::multimap<std::string, std::pair<long, long>> *sparseIndex;
    std::string filePath;
};
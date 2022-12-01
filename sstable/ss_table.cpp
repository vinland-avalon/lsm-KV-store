/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:33:21
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-01 01:40:42
 * @FilePath: /lsm-KV-store/sstable/ss_table.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include "../command/command.cpp"
#include "../mem_table/mem_table.cpp"
#include "./table_meta_info.cpp"
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
        std::fstream file;
        file.open(_filePath);
        file.seekp(0);
    }
    // todo: single instance mode
    TableMetaInfo *getTableMetaInfo() { return this->tableMetaInfo; }

    /**
     * @description: init from memtable and flush memtable to sstable
     * @param {string} _filePath
     * @param {int} _partitionSize
     * @param {MemTable*} memtable
     * @return {*} the pointer to SsTable
     */
    SsTable *initFromMemTable(std::string _filePath, int _partitionSize, MemTable *memtable) {
        // calculate metadata
        tableMetaInfo->setDataStart(tableFile.tellp());

        // flush to SSD
        json records = json({});
        memtable->reachBegin();
        while (!(memtable->curr())) {
            // key1: {key: "key1",type: "SET", value: "100"}
            json keyedRecord;
            keyedRecord[memtable->curr()->getKey()] = memtable->curr()->toJSON();
            records.push_back(keyedRecord);
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
        tableFile << json::parse(sparseIndex);
        tableMetaInfo->setIndexLen(tableFile.tellp() - tableMetaInfo->getIndexStart());

        // save metadata
        tableMetaInfo->writeToFile(&tableFile);

        return this;
    }
    /**
     * @description: from SSD, restore ssTable to memory
     * @description: only load metadata and sparse index
     * @param {string} _filePath
     * @return {*}
     */
    SsTable *initFromFile(std::string _filePath) {
        // load metadata
        this->tableMetaInfo = new TableMetaInfo();
        tableMetaInfo->readFromFile(&(this->tableFile));
        // load sparse index
        // std::string tmpRecords;
        json tmpJSONRecords;
        (this->tableFile).read((char *)&tmpJSONRecords, sizeof(long));
        sparseIndex->clear();
        for (json::iterator it = tmpJSONRecords.begin(); it != tmpJSONRecords.end(); ++it) {
            std::cout << it.key() << " : " << it.value() << "\n";
            sparseIndex->emplace(it.key(), std::pair<long, long>(it.value().at(0), it.value().at(1)));
        }
    }
    Command *query(std::string key) {}
    // void writeRecords(json records) {}
    /**
     * @description: write records to SSD, clear records in JSON, and then append sparse index entry to this instance
     * @param {json} *records
     * @return {*}
     */
    void writeToSSDandClearAndAppendSparseIndex(json *records) {
        std::string key = records->at(0)["key"];
        long start = tableFile.tellp();
        tableFile << records;
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
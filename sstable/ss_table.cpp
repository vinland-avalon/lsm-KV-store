/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:33:21
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-11-30 21:21:52
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
    SsTable(std::string _filePath, int _partitionSize) {}
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
        // SsTable *ssTable = new SsTable(_filePath, _partitionSize);
        // calculate metadata
        tableMetaInfo->setDataStart(tableFile.tellg());
        
        // flush to SSD
        json records = json({});
        memtable->reachBegin();
        while (!(memtable->curr())) {
            json keyedRecord;
            keyedRecord[memtable->curr()->getKey()]=memtable->curr()->toJSON();
            records.push_back(keyedRecord);
            memtable->next();
            if (records.size() >= tableMetaInfo->getPartitionSize()) {
                writeRecords(records);
                appendSparseIndex(records);
                records.clear();
            }
        }
        if (records.size() > 0) {
            writeRecords(records);
        }

        // calculate metadata
        long dataPartLen = tableFile.tellg() - tableMetaInfo->getDataStart();
        tableMetaInfo->setDataLen(dataPartLen);

        // save sparse index and calculate metadata
        tableMetaInfo->setIndexStart(tableFile.tellg());
        tableFile << json::parse(sparseIndex);
        tableMetaInfo->setIndexLen(tableFile.tellg()-tableMetaInfo->getIndexStart());

        // save metadata
        tableMetaInfo->writeToFile(&tableFile);
    }
    SsTable initFromFile(std::string _filePath) {}
    Command query(std::string key) {}
    void writeRecords(json records) {}
    void appendSparseIndex(json records){}

  private:
    TableMetaInfo *tableMetaInfo;
    std::fstream tableFile;
    std::map<std::string, std::pair<long, long>> sparseIndex;
    std::fstream file;
    std::string filePath;
};
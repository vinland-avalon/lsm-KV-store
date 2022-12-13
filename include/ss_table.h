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
        table_file_stream_.close();
    }
    // todo: single instance mode
    std::shared_ptr<TableMetaInfo> GetTableMetaInfo() { return table_meta_info_; }

    static std::shared_ptr<SsTable> InitFromMemTableAndFlushToSSD(std::shared_ptr<MemTable> mem_table, int partition_size, std::string file_path) {
        try {
            if (mem_table == nullptr) {
                spdlog::error("[SsTable][InitFromMemTableAndFlushToSSD] *mem_table is nullptr");
                throw "*mem_table is nullptr";
            }
            std::shared_ptr<SsTable> ssTable = std::shared_ptr<SsTable>(new SsTable(file_path, partition_size));
            ssTable->InitFromMemTable(mem_table);
            return ssTable;
        } catch (...) {
            throw;
        }
    }

    static std::shared_ptr<SsTable> InitFromSSD(std::string file_path) {
        try {
            std::shared_ptr<SsTable> ss_table = std::shared_ptr<SsTable>(new SsTable());
            ss_table->ReloadWithGivenSSDFile(file_path);
            return ss_table;
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
    std::shared_ptr<Command> Query(std::string key) {
        // start, len
        std::pair<long, long> lower_bound_block(-1, -1);
        for (auto it = sparse_index_->begin(); it != sparse_index_->end(); it++) {
            if (it->first <= key) {
                lower_bound_block.first = it->second.first;
                lower_bound_block.second = it->second.second;
            }
        }
        if (lower_bound_block.first == -1) {
            spdlog::info("[SsTable][Query] not even found right block in this block");
            return nullptr;
        }

        // load sparse index
        char buffer[1000];
        table_file_stream_.seekg(lower_bound_block.first);
        table_file_stream_.read(buffer, lower_bound_block.second);
        // std::string blockString = buffer;
        // {"key1":{"key":"key1","type":"SET","value":"100"}}{"key2":{"key":"key2","type":"SET","value":"101"}}
        json block_JSON = json::parse(buffer);
        spdlog::info("[SsTable][Query] the needed command maybe in this block, data:{}", block_JSON.dump());

        for (json::iterator it = block_JSON.begin(); it != block_JSON.end(); ++it) {
            // std::cout << "[query] reload command in block: " << it.key() << " : " << it.value() << "\n";
            if (it.key() == key) {
                spdlog::info("[SsTable][Query] get the needed command:{}", it.value().dump());
                return JSONtoCommand(it.value());
            }
        }
        return nullptr;
    }

    // void writecommands(json commands) {}

  private:
    std::shared_ptr<TableMetaInfo> table_meta_info_;
    std::fstream table_file_stream_;
    // sparse_index_:{ key: {start, len}}
    std::shared_ptr<std::map<std::string, std::pair<long, long>>> sparse_index_;
    std::string file_path_;

    SsTable(std::string file_path, int partition_size) {
        try {
            table_meta_info_ = std::shared_ptr<TableMetaInfo>(new TableMetaInfo());
            table_meta_info_->SetPartitionSize(partition_size);
            file_path_ = file_path;
            sparse_index_ = std::shared_ptr<std::map<std::string, std::pair<long, long>>>(new std::map<std::string, std::pair<long, long>>());
            OpenFileAndCreateOneWhenNotExist(&(table_file_stream_), file_path_);
            // table_file_stream_.open(_filePath, std::ios::in | std::ios::out | std::ios::binary);
            if (!table_file_stream_.is_open()) {
                spdlog::error("[SsTable][Constructor] fail to open file {}", file_path_);
                throw "Fail to open file";
            }
            table_file_stream_.seekp(0);
            table_file_stream_.seekg(0);
            spdlog::info("[SsTable][Constructor] initialized an instance, file_path_: {}, metainfo.partition_size: {}", file_path_, table_meta_info_->GetPartitionSize());
        } catch (std::string error) {
            throw error;
        } catch (std::exception &error) {
            table_file_stream_.close();
            throw error;
        }
    }

    SsTable(){
        table_meta_info_ = std::shared_ptr<TableMetaInfo>(new TableMetaInfo());
        sparse_index_ = std::shared_ptr<std::map<std::string, std::pair<long, long>>>(new std::map<std::string, std::pair<long, long>>());
    }

    /**
     * @description: init from memtable and flush memtable to sstable
     * @param {string} _filePath
     * @param {int} _partitionSize
     * @param {MemTable*} memtable
     * @return {*} the pointer to SsTable
     */
    void InitFromMemTable(std::shared_ptr<MemTable> mem_table) {
        // calculate metadata
        table_meta_info_->SetCommandsStart(table_file_stream_.tellp());

        // flush to SSD, part by part
        json commands;
        mem_table->ReachBegin();
        while (mem_table->Curr()) {
            commands[mem_table->Curr()->GetKey()] = mem_table->Curr()->ToJSON();
            // std::cout << " get record: " << commands << std::endl;
            spdlog::info("[SsTable][initFromMemTable] get command {} from immutableMemTable", mem_table->Curr()->ToJSON().dump());
            mem_table->Next();
            if (commands.size() >= table_meta_info_->GetPartitionSize()) {
                WriteToSSDandClearAndAppendSparseIndex(&commands);
            }
        }
        if (commands.size() > 0) {
            WriteToSSDandClearAndAppendSparseIndex(&commands);
        }

        // calculate metadata
        long dataPartLen = table_file_stream_.tellp() - table_meta_info_->GetCommandsStart();
        table_meta_info_->SetCommandsLen(dataPartLen);

        // save sparse index and calculate metadata
        table_meta_info_->SetIndexStart(table_file_stream_.tellp());
        json sparse_index_JSON;
        for (auto single_sparse_index_JSON : (*sparse_index_)) {
            // key:{start, len}
            sparse_index_JSON[single_sparse_index_JSON.first] = (json{single_sparse_index_JSON.second.first, single_sparse_index_JSON.second.second});
        }
        std::string sparse_index_string = sparse_index_JSON.dump();
        spdlog::info("[SsTable][InitFromMemTable] sparse index: {}", sparse_index_string);
        WriteStringToFile(sparse_index_string, &table_file_stream_);
        // tableFile.write(sparseIndexString.c_str(), sparseIndexString.size() + 1);
        // tableFile << sparse_index_;
        table_meta_info_->SetIndexLen(table_file_stream_.tellp() - table_meta_info_->GetIndexStart());

        // save metadata
        table_meta_info_->WriteToFile(&table_file_stream_);

        table_file_stream_.flush();
    }

    /**
     * @description: from SSD, restore ssTable to memory
     * @description: only load metadata and sparse index
     * @param {string} _filePath
     * @return {*}
     */
    void ReloadWithGivenSSDFile(std::string file_path) {
        // bound to SSD
        file_path_ = file_path;
        OpenFileAndCreateOneWhenNotExist(&(table_file_stream_), file_path_);
        if (!table_file_stream_.is_open()) {
            spdlog::error("[SsTable][ReloadWithGivenSSDFile] fail to open file {}", file_path_);
            throw "Fail to open file";
        }
        table_file_stream_.seekp(0);
        table_file_stream_.seekg(0);
        spdlog::info("[SsTable][ReloadWithGivenSSDFile] initialized an instance, file_path_: {}, metainfo.partition_size: {}", file_path_, table_meta_info_->GetPartitionSize());
        
        // load metadata
        table_meta_info_->ReadFromFile(&(table_file_stream_));

        // load sparse index
        char buffer[1000];
        table_file_stream_.seekg(table_meta_info_->GetIndexStart());
        table_file_stream_.read(buffer, table_meta_info_->GetIndexLen());

        // std::string sparseIndexString = buffer;

        // {"key1":[0,50]}
        // // std::cout << "lll" << sparseIndexString << std::endl;

        // json tmpJSONcommands = json::parse(sparseIndexString);
        json tmpJSONcommands = json::parse(buffer);
        spdlog::info("[SsTable][initFromFile] reload sparse index {}", tmpJSONcommands.dump());

        for (json::iterator it = tmpJSONcommands.begin(); it != tmpJSONcommands.end(); ++it) {
            // std::cout << "[initFromFile] reload sparse index: " << it.key() << " : " << it.value() << "\n";
            sparse_index_->emplace(it.key(), std::pair<long, long>(it.value().at(0), it.value().at(1)));
        }
    }

    /**
     * @description: write commands to SSD, clear commands in JSON, and then append sparse index entry to this instance
     * @param {json} *commands
     * @return {*}
     */
    void WriteToSSDandClearAndAppendSparseIndex(json *commands) {
        spdlog::info("[SsTable][writeToSSDandClearAndAppendSparseIndex] write commands: {}", commands->dump());
        std::string key = (*(commands->begin()))["key"];
        long start = table_file_stream_.tellp();

        WriteStringToFile(commands->dump(), &table_file_stream_);

        commands->clear();

        long len = table_file_stream_.tellp() - start;
        sparse_index_->emplace(key, std::pair<long, long>(start, len));
    }
};

#endif
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
#include "mem_table_skip_list_impl.h"
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
    LsmKvStore(std::string data_dir_path, int mem_threshold, int partition_size) : data_dir_path_(data_dir_path), mem_threshold_(mem_threshold), partition_size_(partition_size) {
        try {
            rw_lock_ptr_ = new std::shared_mutex();
            ss_tables_ = new std::list<std::shared_ptr<SsTable>>();
            // mem_table_ = std::shared_ptr<MemTable>(new RedBlackTreeMemTable());
            mem_table_ = std::shared_ptr<MemTable>(new SkipListMemTable(10));
            if (!IsSolidDirectory(data_dir_path_)) {
                spdlog::warn("[LsmKvStore][constructor] dataDir: {} is invalid", this->data_dir_path_);
                // std::string will cast to std::exception
                throw data_dir_path_ + " not valid";
            }
            std::vector<std::string> files_names_in_dir = GetFilenamesInDirectory(data_dir_path_);
            // make it order: from big to small
            std::map<long, std::shared_ptr<SsTable>, std::greater<long>> ss_table_map;
            for (std::string file_name : files_names_in_dir) {
                std::string file_path = data_dir_path_ + "/" + file_name;
                // it's a SSTable file, it's like 142153253253.table
                if (EndsWith(file_name, TABLE_SUFFIX)) {
                    spdlog::info("[LsmKvStore][constructor] init ssTable from file: {}", file_name);
                    long time = getTimeFromSSTableName(file_name);
                    std::shared_ptr<SsTable> ss_table = SsTable::InitFromSSD(file_path);
                    ss_table_map.insert(std::make_pair(time, ss_table));
                    // WAL_TMP: restore from walTmp file, and such file commonly derive from ssTable that fails when being persisted
                    // WAL: restore from a pre-exist file, can keep on using it
                } else if (file_name == WAL || file_name == WAL_TMP) {
                    spdlog::info("[LsmKvStore][constructor] restore commands from file: {}", file_name);
                    std::fstream *tmp_fstream_ptr = new std::fstream;
                    OpenFileAndCreateOneWhenNotExist(tmp_fstream_ptr, file_path);
                    // tmpFstream->open(fileWithPath, std::ios::in | std::ios::out | std::ios::binary);
                    if (!tmp_fstream_ptr->is_open()) {
                        spdlog::info("[LsmKvStore][constructor] fail to open file: {}", file_name);
                        throw "fail to open file";
                    }
                    // such wal file can continue to work
                    if (file_name == WAL) {
                        wal_file_path_ = file_path;
                        wal_file_stream_ptr_ = tmp_fstream_ptr;
                    }
                    RestoreFromWal(tmp_fstream_ptr);
                }
            }
            // wal file not exist, so just create one. It's normal, if there's not crush before.
            if (wal_file_path_ == "" || wal_file_stream_ptr_ == nullptr) {
                wal_file_path_ = data_dir_path_ + "/" + WAL;
                wal_file_stream_ptr_ = new std::fstream;
                OpenFileAndCreateOneWhenNotExist(wal_file_stream_ptr_, wal_file_path_);
            }
            for (auto it = ss_table_map.begin(); it != ss_table_map.end(); it++) {
                ss_tables_->push_back(it->second);
            }
        } catch (std::exception &error) {
            spdlog::error("[LsmKvStore][constructor] error: {}", error.what());
            throw error;
        }
    }

    void Set(std::string key, std::string value) {
        try {
            auto command = std::shared_ptr<SetCommand>(new SetCommand("SET", key, value));
            ExecuteCommand(command);
        } catch (std::exception &error) {
            // the toppest function, so won't throw again
            spdlog::error("[LsmKvStore][Set] error: {}", error.what());
        }
    }

    void Remove(std::string key) {
        try {
            auto command = std::shared_ptr<RmCommand>(new RmCommand("RM", key));
            ExecuteCommand(command);
        } catch (std::exception &error) {
            // the toppest function, so won't throw again
            spdlog::error("[LsmKvStore][Rm] error: {}", error.what());
        }
    }

    std::string Get(std::string key) {
        std::shared_lock<std::shared_mutex> r_lock(*rw_lock_ptr_);
        // try to get corresponding command in memTable
        std::shared_ptr<Command> command = mem_table_->Get(key);
        // try to get corresponding command in immutableMemTable
        if (command == nullptr && immutable_mem_table_ != nullptr) {
            command = immutable_mem_table_->Get(key);
        }
        // try to get corresponding command in ssTables
        if (command == nullptr) {
            for (std::shared_ptr<SsTable> ss_table : *ss_tables_) {
                command = ss_table->Query(key);
                if (command) {
                    break;
                }
            }
        }
        // check the "get" result and return value;
        if (command == nullptr)
            return "";
        else if (command->GetType() == "SET") {
            std::string value = (std::dynamic_pointer_cast<SetCommand>(command))->GetValue();
            spdlog::info("[LsmKvStore][Get] get {} for key: {}", value, key);
            return value;
        } else if (command->GetType() == "RM") {
            return "";
        }
        return "";
    }

    ~LsmKvStore() {
        FlushSSTableAndWal();
        delete wal_file_stream_ptr_;
        delete rw_lock_ptr_;
        delete ss_tables_;
    }

  private:
    std::shared_ptr<MemTable> mem_table_;
    std::shared_ptr<MemTable> immutable_mem_table_;

    // std::list<SsTable *> *ssTables;
    std::list<std::shared_ptr<SsTable>> *ss_tables_;
    const std::string data_dir_path_;
    const int mem_threshold_;
    std::shared_mutex *rw_lock_ptr_;
    const int partition_size_;
    std::string wal_file_path_ = "";
    std::fstream *wal_file_stream_ptr_ = nullptr;

    /**
     * @description: wal/tmp_file data structure: len json len json
     * @param {fstream} *fileStream
     * @return {*}
     */
    void RestoreFromWal(std::fstream *file_stream) {
        file_stream->seekg(0);
        long len = 0;
        char buffer[1000];
        while (file_stream->eof()) {
            file_stream->read((char *)&len, sizeof(len));
            file_stream->read(buffer, len);
            std::string commandString = buffer;
            auto command = JSONtoCommand(json::parse(commandString));
            mem_table_->Set(command->GetKey(), command);
        }
    }
    /**
     * @description: 1. memTable->immutableMemTable
     * @discription: 2. old walfile rename to WAL_TMP, create new walFile
     * @return {*}
     */
    void SwitchMemTableAndWal() {
        // switch MemTable
        immutable_mem_table_ = mem_table_;
        mem_table_ = std::shared_ptr<MemTable>(new SkipListMemTable(10));
        // switch WAL file
        wal_file_stream_ptr_->close();
        std::string wal_tmp_file_path = data_dir_path_ + "/" + WAL_TMP;
        // If there is wal_tmp file exsiting, it is left before, just delete it.
        if (IsFileExisting(wal_tmp_file_path)) {
            if (!std::remove(wal_tmp_file_path.c_str())) {
                spdlog::error("[LsmKvStore][SwitchMemTableAndWal] fail to delete walTmp: {}", wal_tmp_file_path);
                throw "fail to delete WAL_TMP";
            }
        }
        if (std::rename(this->wal_file_path_.c_str(), wal_tmp_file_path.c_str())) {
            spdlog::error("[LsmKvStore][switchMemTableAndWal] fail to rename walTmp: {} to {}", wal_file_path_, wal_tmp_file_path);
            throw "fail to rename WAL to WAL_TMP";
        }
        spdlog::info("[LsmKvStore][SwitchMemTableAndWal] rename walTmp: {} to {}", wal_file_path_, wal_tmp_file_path);
        // create a new wal file
        OpenFileAndCreateOneWhenNotExist(wal_file_stream_ptr_, wal_file_path_);
    }
    /**
     * @description: 1. flush immutableMemTable to brand-new file, created in the form of dataDir + system time + table_suffix
     * @discription: 2. add the ssTable structure to list
     * @discription: 3. delete old immutableMemTable and WAL_TMP
     * @return {*}
     */
    void FlushToSSD() {
        std::string ss_table_path = data_dir_path_ + "/" + GetSystemTimeInMills() + TABLE_SUFFIX;
        std::shared_ptr<SsTable> ss_table = SsTable::InitFromMemTableAndFlushToSSD(immutable_mem_table_, partition_size_, ss_table_path);
        
        ss_tables_->insert(ss_tables_->begin(), ss_table);

        immutable_mem_table_ = nullptr;
        std::string old_wal_tmp = data_dir_path_ + "/" + WAL_TMP;
        if (IsFileExisting(old_wal_tmp)) {
            if (std::remove(old_wal_tmp.c_str())) {
                spdlog::error("[LsmKvStore][FlushToSsTable] fail to delete old_wal_tmp: {}", old_wal_tmp);
            }
        }
    }

    void ExecuteCommand(std::shared_ptr<Command> command) {
        try {
            // todo: take the advantage of thread pool
            std::string command_string = command->ToJSON().dump();
            long len = command_string.size();
            std::unique_lock<std::shared_mutex> w_lock(*rw_lock_ptr_);

            // write ahead log
            wal_file_stream_ptr_->write((const char *)&len, sizeof(len));
            WriteStringToFile(command_string, wal_file_stream_ptr_);

            // memtable
            mem_table_->Set(command->GetKey(), command);

            // if memtable is at threshold, make it consistent to SSD
            if (mem_table_->Size() > mem_threshold_) {
                FlushSSTableAndWal();
            }
        } catch (std::exception &error) {
            spdlog::error("[LsmKvStore][ExecuteCommand] error: {}", error.what());
            throw;
        }
    }

    void FlushSSTableAndWal() {
        try {
            // memTable -> immutableMemTable
            // new memTable
            SwitchMemTableAndWal();
            // todo: make the flush action to async
            FlushToSSD();
        } catch (std::exception &error) {
            spdlog::error("[LsmKvStore][flushSSTableAndWal] error: {}", error.what());
            throw;
        }
    }

    long getTimeFromSSTableName(std::string file_name) {
        int start = 0;
        // drop .table
        int len = file_name.size() - TABLE_SUFFIX.size() - 1;
        long time = stol(file_name.substr(start, len));
        return time;
    }

  protected:
};

#endif
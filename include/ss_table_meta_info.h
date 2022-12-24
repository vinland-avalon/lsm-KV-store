/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 12:10:48
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-02 15:20:00
 * @FilePath: /lsm-KV-store/sstable/table_meta_info.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#ifndef _SS_TABLE_META_INFO_H_
#define _SS_TABLE_META_INFO_H_

#include <fstream>
#include <iostream>

class TableMetaInfo {
  private:
    long version_;
    long commands_start_;
    long commands_len_;
    long index_start_;
    long index_len_;
    long partition_size_;
    long filter_start_;
    long filter_len_;

  public:
    /**
     * @description: write metacommands to file
     * @param {fstream} *file
     * @return {*}
     */
    void WriteToFile(std::fstream *file) {
        (*file).write((const char *)&partition_size_, sizeof(partition_size_));
        (*file).write((const char *)&commands_start_, sizeof(commands_start_));
        (*file).write((const char *)&commands_len_, sizeof(commands_len_));
        (*file).write((const char *)&index_start_, sizeof(index_start_));
        (*file).write((const char *)&index_len_, sizeof(index_len_));
        (*file).write((const char *)&version_, sizeof(version_));
        (*file).write((const char *)&filter_start_, sizeof(filter_start_));
        (*file).write((const char *)&filter_len_, sizeof(filter_len_));
        file->clear();
    }
    /**
     * @description: init a table_meta_info_ structure from file;
     * @param {fstream*} file
     * @return {*}
     */
    void ReadFromFile(std::fstream *file) {
        long len = CalLenOfMetaInfo();
        file->seekg(-len, std::ios::end);
        (*file).read((char *)&partition_size_, sizeof(partition_size_));
        (*file).read((char *)&commands_start_, sizeof(commands_start_));
        (*file).read((char *)&commands_len_, sizeof(commands_len_));
        (*file).read((char *)&index_start_, sizeof(index_start_));
        (*file).read((char *)&index_len_, sizeof(index_len_));
        (*file).read((char *)&version_, sizeof(version_));
        (*file).read((char *)&filter_start_, sizeof(filter_start_));
        (*file).read((char *)&filter_len_, sizeof(filter_len_));
    }

    long CalLenOfMetaInfo() const {
        return sizeof(long) * 8;
    }
    // TableMetaInfo(std::fstream* file){}
    // TableMetaInfo(){}
    long GetPartitionSize() const {
        return partition_size_;
    }
    void SetPartitionSize(long partition_size) {
        partition_size_ = partition_size;
    }
    long GetCommandsStart() const {
        return commands_start_;
    }
    void SetCommandsStart(long commands_start) {
        commands_start_ = commands_start;
    }
    long GetCommandsLen() const {
        return commands_len_;
    }
    void SetCommandsLen(long commands_len) {
        commands_len_ = commands_len;
    }
    long GetIndexLen() const {
        return index_len_;
    }
    void SetIndexLen(long index_len) {
        index_len_ = index_len;
    }
    long GetIndexStart() const {
        return index_start_;
    }
    void SetIndexStart(long index_start) {
        index_start_ = index_start;
    }
    void SetFilterStart(long filter_start) {
        filter_start_ = filter_start;
    }
    long GetFilterStart() const {
        return filter_start_;
    }
    void SetFilterLen(long filter_len) {
        filter_len_ = filter_len;
    }
    long GetFilterLen() const {
        return filter_len_;
    }
};

#endif
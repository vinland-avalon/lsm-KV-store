/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 12:10:48
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-11-30 21:35:19
 * @FilePath: /lsm-KV-store/sstable/table_meta_info.cpp
 * @Description: 
 * 
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved. 
 */
#include <fstream>

class TableMetaInfo {
private:
    long version;
    long dataStart;
    long dataLen;
    long indexStart;
    long indexLen;
    long partitionSize;


public:
    void writeToFile(std::fstream *file){}
    static TableMetaInfo readFromFile(std::fstream* file){}
    TableMetaInfo(std::fstream* file){}
    TableMetaInfo(){}
    long getPartitionSize() {
        return this->partitionSize;
    }
    void setPartitionSize(long partitionSize) {
        this->partitionSize = partitionSize;
    }
    long getDataStart() {
        return this->dataStart;
    }
    void setDataStart(long dataStart) {
        this->dataStart = dataStart;
    }
    long getDataLen() {
        return this->dataLen;
    }
    void setDataLen(long dataLen) {
        this->dataLen = dataLen;
    }
    long getIndexLen() {
        return this->indexLen;
    }
    void setIndexLen(long indexLen) {
        this->indexLen = indexLen;
    }
    long getIndexStart() {
        return this->indexStart;
    }
    void setIndexStart(long indexStart) {
        this->indexStart = indexStart;
    }


};
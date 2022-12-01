/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 12:10:48
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-01 00:03:09
 * @FilePath: /lsm-KV-store/sstable/table_meta_info.cpp
 * @Description: 
 * 
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved. 
 */
#include <fstream>
#include <iostream>

class TableMetaInfo {
private:
    long version;
    long dataStart;
    long dataLen;
    long indexStart;
    long indexLen;
    long partitionSize;


public:
    /**
     * @description: write metadata to file
     * @param {fstream} *file
     * @return {*}
     */
    void writeToFile(std::fstream *file){
        (*file).write((const char*)&partitionSize, sizeof(long));
        (*file).write((const char*)&dataStart, sizeof(long));
        (*file).write((const char*)&dataLen, sizeof(long));
        (*file).write((const char*)&indexStart, sizeof(long));
        (*file).write((const char*)&indexLen, sizeof(long));
        (*file).write((const char*)&version, sizeof(long));
    }
    /**
     * @description: init a tableMetaInfo structure from file;
     * @param {fstream*} file
     * @return {*}
     */
    void readFromFile(std::fstream* file){
        long len = calLenOfMetaInfo();
        file->seekg(-len,std::ios::end);
        (*file).read((char*)&partitionSize, sizeof(long));
        (*file).read((char*)&dataStart, sizeof(long));
        (*file).read((char*)&dataLen, sizeof(long));
        (*file).read((char*)&indexStart, sizeof(long));
        (*file).read((char*)&indexLen, sizeof(long));
        (*file).read((char*)&version, sizeof(long));
    }

    long calLenOfMetaInfo(){
        return sizeof(long)*6;
    }
    // TableMetaInfo(std::fstream* file){}
    // TableMetaInfo(){}
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
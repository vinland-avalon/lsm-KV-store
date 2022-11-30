/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 12:10:48
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-11-30 14:31:26
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
    void writeToFile(std::fstream* file){}
    static TableMetaInfo readFromFile(std::fstream* file){}
    TableMetaInfo(std::fstream* file){}
    TableMetaInfo(){}
};
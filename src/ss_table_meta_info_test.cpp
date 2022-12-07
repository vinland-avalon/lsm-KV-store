/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 23:02:04
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-07 17:27:24
 * @FilePath: /lsm-KV-store/src/ss_table_meta_info_test.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include "ss_table_meta_info.h"
#include <iostream>

// g++ -o table_meta_info_test ./table_meta_info_test.cpp ../table_meta_info.cpp
// ./table_meta_info_test
int main() {
    TableMetaInfo *info = new TableMetaInfo();
    info->setDataLen(1);
    info->setDataStart(2);
    info->setIndexLen(3);
    info->setIndexStart(4);
    info->setPartitionSize(5);

    std::fstream file;
    file.open("./ssd_for_table_meta_info_test", std::ios::binary | std::ios::out);
    file.write("iambohanwuandyou?", 15);
    info->writeToFile(&file);
    file.close();

    file.open("./ssd_for_table_meta_info_test", std::ios::binary | std::ios::in);
    TableMetaInfo *info2 = new TableMetaInfo();
    info2->readFromFile(&file);
    std::cout << info2->getDataLen() << '\t' << info2->getDataStart() << std::endl;
    file.close();
}
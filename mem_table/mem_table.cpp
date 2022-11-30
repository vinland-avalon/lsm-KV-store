/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:54:25
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-11-30 14:31:09
 * @FilePath: /lsm-KV-store/mem_table/mem_table.cpp
 * @Description: 
 * 
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved. 
 */
#include <string>

class MemTable {
public:
    virtual std::string get(std::string key) = 0;
    virtual void set(std::string key, std::string value) = 0;
    virtual void remove(std::string key) = 0;
};
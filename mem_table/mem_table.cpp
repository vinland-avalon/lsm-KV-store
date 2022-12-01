/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:54:25
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-11-30 18:45:29
 * @FilePath: /lsm-KV-store/mem_table/mem_table.cpp
 * @Description: 
 * 
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved. 
 */
#include <string>
#include "../command/command.cpp"

// iterator
class MemTable {
public:
    virtual std::string get(std::string key) = 0;
    virtual void set(std::string key, std::string value) = 0;
    virtual void remove(std::string key) = 0;
    
    // features about iterator
    virtual void reachBegin() = 0;
    virtual Command* curr() = 0;
    virtual Command* next() = 0;
};
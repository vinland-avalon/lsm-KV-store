/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:54:25
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-05 22:18:29
 * @FilePath: /lsm-KV-store/mem_table/mem_table.h
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#ifndef _Mem_Table_H_
#define _Mem_Table_H_
#include "../command/command.h"
#include <string>

// iterator
class MemTable {
  public:
    virtual Command *get(std::string key) = 0;
    virtual void set(std::string key, Command *command) = 0;
    virtual void remove(std::string key) = 0;
    virtual long size() = 0;

    // features about iterator
    virtual void reachBegin() = 0;
    virtual Command *curr() = 0;
    virtual void next() = 0;
};

#endif
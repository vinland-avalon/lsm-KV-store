/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:54:25
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-11 11:29:56
 * @FilePath: /lsm-KV-store/include/mem_table.h
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#ifndef _MEM_TABLE_H_
#define _MEM_TABLE_H_

#include "command.h"
#include <string>

// iterator
class MemTable {
  public:
    virtual std::shared_ptr<Command> Get(std::string key) const = 0;
    virtual void Set(std::string key, std::shared_ptr<Command> command) = 0;
    // remove: in fact, won't be called
    virtual void Remove(std::string key) = 0;
    virtual long Size() const = 0;

    // features about iterator
    // since they are only used when flushing immmutable memtable to SSD, it is not necessary to consider concurrency
    virtual void ReachBegin() = 0;
    virtual std::shared_ptr<Command> Curr() const = 0;
    virtual void Next() = 0;
};

#endif
/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:53:41
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-11 11:11:03
 * @FilePath: /lsm-KV-store/include/mem_table_red_black_tree_impl.h
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#ifndef _MEM_TABLE_RED_BLACK_TREE_IMPL_H_
#define _MEM_TABLE_RED_BLACK_TREE_IMPL_H_

#include "command.h"
#include "mem_table.h"
#include <map>

class RedBlackTreeMemTable : public MemTable {
  public:
    RedBlackTreeMemTable() {
        this->table = new std::map<std::string, std::shared_ptr<Command>>();
    }
    std::shared_ptr<Command> Get(std::string key) const {
        auto it = table->find(key);
        if (it == table->end()) {
            return nullptr;
        }
        return it->second;
    }
    void Set(std::string key, std::shared_ptr<Command> command) {
        table->insert(std::make_pair(key, command));
    }
    void Remove(std::string key) {
        table->erase(key);
    }

    // features about iterator
    void ReachBegin() {
        this->it = table->begin();
    };
    std::shared_ptr<Command> Curr() const {
        if (it == table->end()) {
            return nullptr;
        } else {
            return it->second;
        }
    };
    void Next() {
        it++;
    };
    // todo: cal size in bytes
    long Size() const {
        return table->size();
    }

  private:
    std::map<std::string, std::shared_ptr<Command> > *table;
    std::map<std::string, std::shared_ptr<Command>>::iterator it;
};

#endif
/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:53:41
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-02 12:01:14
 * @FilePath: /lsm-KV-store/mem_table/red_black_tree_impl.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include "../command/command.h"
#include "./mem_table.h"
#include <map>

class RedBlackTreeMemTable : public MemTable {
  public:
    RedBlackTreeMemTable() {
        this->table = new std::multimap<std::string, Command *>();
    }
    Command *get(std::string key) {
        it = table->find(key);
        if (it == table->end()) {
            return nullptr;
        }
        return it->second;
    }
    void set(std::string key, Command *command) {
        table->insert(std::make_pair(key, command));
    }
    void remove(std::string key) {
        table->erase(key);
    }
    // features about iterator
    void reachBegin() {
        this->it = table->begin();
    };
    Command *curr() {
        if (it == table->end()) {
            return nullptr;
        } else {
            return it->second;
        }
    };
    void next() {
        it++;
    };

  private:
    std::multimap<std::string, Command *> *table;
    std::multimap<std::string, Command *>::iterator it;
};
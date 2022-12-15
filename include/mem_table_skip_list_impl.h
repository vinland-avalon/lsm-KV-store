/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:55:17
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-01 01:40:34
 * @FilePath: /lsm-KV-store/mem_table/skip_list_impl.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#ifndef _SKIP_LIST_MEM_TABLE_H_
#define _SKIP_LIST_MEM_TABLE_H_

#include "command.h"
#include "skip_list.h"
#include "skip_list_iter.h"
#include <string>

class SkipListMemTable : public MemTable {
  public:
    SkipListMemTable(int max_level = 10) {
        max_level_ = max_level;
        skip_list_ = std::shared_ptr<SkipList<std::string, std::shared_ptr<Command>>>(new SkipList<std::string, std::shared_ptr<Command>>(max_level_));
    }
    std::shared_ptr<Command> Get(std::string key) const {
        return skip_list_->search_element(key);
    }
    void Set(std::string key, std::shared_ptr<Command> command) {
        skip_list_->insert_element(key, command);
    }
    // remove: in fact, won't be called
    void Remove(std::string key) {
        skip_list_->delete_element(key);
    }
    long Size() const {
        return skip_list_->size();
    }

    // features about iterator
    // since they are only used when flushing immmutable memtable to SSD, it is not necessary to consider concurrency
    void ReachBegin() {
        iter_ = ListIter<Node<std::string, std::shared_ptr<Command>>>(skip_list_->front());
    }
    std::shared_ptr<Command> Curr() const {
        if (iter_ == nullptr) {
            return nullptr;
        }
        return iter_.Iter->get_value();
    }
    void Next() {
        ++iter_;
    }

  private:
    std::shared_ptr<SkipList<std::string, std::shared_ptr<Command>>> skip_list_ = nullptr;
    int max_level_;
    ListIter<Node<std::string, std::shared_ptr<Command>>> iter_;
};

#endif
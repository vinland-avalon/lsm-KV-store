/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:53:41
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-11-30 14:31:13
 * @FilePath: /lsm-KV-store/mem_table/red_black_tree_impl.cpp
 * @Description: 
 * 
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved. 
 */
#include "./mem_table.cpp"
#include <map>
#include "../command/command.cpp"

class RedBlackTreeMemTable: public MemTable {
public:
    std::string get(std::string key){}
    void set(std::string key, std::string value){}
    void remove(std::string key){}
private:
    std::map<std::string, Command> table;
};
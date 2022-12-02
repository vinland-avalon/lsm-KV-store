/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-01 00:44:43
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-02 22:46:41
 * @FilePath: /lsm-KV-store/sstable/test_and_bin/ss_table_test.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include "../../mem_table/red_black_tree_impl.cpp"
#include "../ss_table.cpp"
#include <iostream>

// cmake --build .
// ./Test_Ss_Table
// exe: Test_Ss_Table
// ssd: sstable_1

int main() {
    // memtable
    MemTable *memTable = new RedBlackTreeMemTable();
    Command *command = new SetCommand("SET", "key1", "100");
    Command *command2 = new SetCommand("SET", "key2", "101");
    memTable->set("key1", command);
    memTable->set("key2", command2);
    std::cout << memTable->get("key1")->toJSON() << std::endl;
    std::cout << memTable->curr()->toJSON() << std::endl;
    memTable->next();
    std::cout << memTable->curr() << std::endl;
    memTable->reachBegin();

    // SsTable
    SsTable *ssTable = new SsTable("./sstable_1", 15);
    ssTable->initFromMemTable(memTable);

    delete ssTable;
    // {"key1":{"key":"key1","type":"SET","value":"100"}}{"key2":{"key":"key2","type":"SET","value":"101"}}{"key1":[0,50]}
    ssTable = new SsTable("./sstable_1", 15);
    ssTable->initFromFile();
    Command *commandValid = ssTable->query("key2");
    if (command) {
        std::cout << "[TEST: ss_table]1 pass:: get " << commandValid->toJSON() << std::endl;
    }
    Command *commandInvalid = ssTable->query("keyInvalid");
    if (!commandInvalid) {
        std::cout << "[TEST: ss_table]2 pass: command not exist" << std::endl;
    }

    return 0;
}
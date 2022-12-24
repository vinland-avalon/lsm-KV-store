/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-09 16:25:45
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-10 16:14:08
 * @FilePath: /lsm-KV-store/test.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#include "bloom_filter.h"
#include "lsm_kv_store.h"
#include "spdlog/spdlog.h"
#include "utils_for_file_operation.h"
#include "utils_for_time_operation.h"
#include <gtest/gtest.h>
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

TEST(UtilsForFileOperation, test) {
    EXPECT_EQ(IsSolidDirectory("."), true);
    EXPECT_EQ(IsSolidDirectory("invalid_dir"), false);
    EXPECT_GT(GetFilenamesInDirectory(".").size(), 0);
    EXPECT_EQ(IsFileExisting("test.cpp"), true);
    EXPECT_EQ(IsFileExisting("invalid_file"), false);
}

TEST(PreWork, ClearDataDirectory) {
    DeleteFilesInDir("./data");
}

TEST(BloomFilter, test) {
    std::string bloom_table;
    std::vector<std::string> keys;
    std::string key1 = "key1";
    std::string key2 = "key2";
    keys.push_back(key1);
    BloomFilter::CalculateFilterTable(keys, &bloom_table);
    spdlog::info("[Test-Variable] BloomFilter bloom_table: {}", bloom_table);
    EXPECT_EQ(BloomFilter::KeyMayExist(key1, bloom_table), true);
    EXPECT_EQ(BloomFilter::KeyMayExist(key2, bloom_table), false);
}

TEST(WholeWorkingFlow, SetKey1) {
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    store->Set("key1", "300");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "300");
    EXPECT_EQ(store->Get("key2"), "");
}

TEST(WholeWorkingFlow, NotSetKey1ButGetItFromSSD) {
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    // store->Set("key1", "300");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "300");
    EXPECT_EQ(store->Get("key2"), "");
}

TEST(WholeWorkingFlow, AgainSetKey1) {
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    store->Set("key1", "400");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "400");
    EXPECT_EQ(store->Get("key2"), "");
}

TEST(WholeWorkingFlow, AgainNotSetKey1ButGetItFromSSD) {
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    // store->Set("key1", "400");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "400");
    EXPECT_EQ(store->Get("key2"), "");
}

TEST(WholeWorkingFlow, RemoveKey1) {
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    store->Remove("key1");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "");
    EXPECT_EQ(store->Get("key2"), "");
}

TEST(WholeWorkingFlow, NotRemoveKey1ButGetItFromSSD) {
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    // store->Remove("key1");
    store->Get("key1");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "");
    EXPECT_EQ(store->Get("key2"), "");
}

TEST(WholeWorkingFlow, LotsOfCommands) {
    DeleteFilesInDir("./data");

    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 10, 5));

    std::vector<std::string> values;
    srand(time(NULL));
    for (int i = 0; i < 56; i++) {
        values.emplace_back(std::to_string(rand()));
    }
    for (auto &value : values) {
        std::string key = "key" + value;
        store->Set(key, value);
    }

    store = nullptr;

    auto ss_table_names = GetFilenamesInDirectory("./data");
    spdlog::info("[Test-Result]: should generate 6 SSTable files, we get: {}", ss_table_names.size());
    EXPECT_EQ(ss_table_names.size(), 6);

    // reload them and do other tests
    store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 10, 5));
    int find = 0;
    for (auto &value : values) {
        std::string key = "key" + value;
        if (store->Get(key) == value) {
            find++;
        }
        key = "key" + std::to_string(rand());
        if (store->Get(key) != "") {
            find++;
        }
    }
    EXPECT_EQ(find, 56);
}

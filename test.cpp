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

#include "utils_for_time_operation.h"
#include "utils_for_file_operation.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <gtest/gtest.h>
#include "spdlog/spdlog.h"
#include "lsm_kv_store.h"
using json = nlohmann::json;

TEST(FooTest, test0) {
    EXPECT_EQ(0, 0);
    EXPECT_EQ("a", "a");
}

TEST(UtilsForFileOperation, test){
    EXPECT_EQ(IsSolidDirectory("."), true);
    EXPECT_EQ(IsSolidDirectory("invalid_dir"), false);
    EXPECT_GT(GetFilenamesInDirectory(".").size(), 0);
    EXPECT_EQ(IsFileExisting("test.cpp"), true);
    EXPECT_EQ(IsFileExisting("invalid_file"), false);
}

TEST(PreWork, ClearDataDirectory){
    DeleteFilesInDir("./data");
}

TEST(WholeWorkingFlow, SetKey1){
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    store->Set("key1", "300");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "300");
    EXPECT_EQ(store->Get("key2"), "");
}

TEST(WholeWorkingFlow, NotSetKey1ButGetItFromSSD){
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    // store->Set("key1", "300");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "300");
    EXPECT_EQ(store->Get("key2"), "");
}

TEST(WholeWorkingFlow, AgainSetKey1){
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    store->Set("key1", "400");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "400");
    EXPECT_EQ(store->Get("key2"), "");
}

TEST(WholeWorkingFlow, AgainNotSetKey1ButGetItFromSSD){
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    // store->Set("key1", "400");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "400");
    EXPECT_EQ(store->Get("key2"), "");
}

TEST(WholeWorkingFlow, RemoveKey1){
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    store->Remove("key1");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "");
    EXPECT_EQ(store->Get("key2"), "");
}

TEST(WholeWorkingFlow, NotRemoveKey1ButGetItFromSSD){
    auto store = std::shared_ptr<LsmKvStore<SkipListMemTable>>(new LsmKvStore<SkipListMemTable>("./data", 2, 1));
    // store->Remove("key1");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
    EXPECT_EQ(store->Get("key1"), "");
    EXPECT_EQ(store->Get("key2"), "");
}



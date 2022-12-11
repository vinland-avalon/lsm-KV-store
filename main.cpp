/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-07 18:04:21
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-10 15:53:06
 * @FilePath: /lsm-KV-store/main.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include "lsm_kv_store.h"
#include "spdlog/spdlog.h"
#include "utils_for_time_operation.h"
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main(int, char **) {
    // json j;
    // j["hello"] = "world";
    // std::cout << "in ./src/utils_for_time_operation.h, getSystemTimeInMills:" << getSystemTimeInMills()
    //           << ", j['Hello'] = " << j["hello"] << std::endl;
    // spdlog::info("Welcome to spdlog!!!!!");
    auto store = std::shared_ptr<LsmKvStore>(new LsmKvStore("./data", 2, 1));
    store->Set("key1", "300");
    spdlog::info("[Test-Result]: try to get key1's value: {}", store->Get("key1"));
    spdlog::info("[Test-Result]: try to get key2's value: {}", store->Get("key2"));
}

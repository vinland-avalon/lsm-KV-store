/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-07 18:04:21
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-07 19:14:58
 * @FilePath: /lsm-KV-store/main.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include <iostream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main(int, char **) {
    json j;
    j["hello"] = "world";
    std::cout << "j['Hello'] = " << j["hello"] << std::endl;
}

/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 17:16:32
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-01 01:40:58
 * @FilePath: /lsm-KV-store/tutorial.cpp
 * @Description: practice CMake
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
// tutorial.cpp

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " number" << std::endl;
        return 1;
    }

    // convert input to double
    const double inputValue = atof(argv[1]);

    // calculate square root
    const double outputValue = sqrt(inputValue);
    std::cout << "The square root of " << inputValue
              << " is " << outputValue
              << std::endl;

    auto config_json = json::parse(R"({"happy": true, "pi": 3.141})");
    std::cout << config_json << std::endl;

    return 0;
}
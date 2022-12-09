/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-09 16:25:45
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-09 16:25:49
 * @FilePath: /lsm-KV-store/test.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#include "utils_for_time_operation.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <gtest/gtest.h>
using json = nlohmann::json;

TEST(FooTest, test0) {
    EXPECT_EQ(0, 0);
    EXPECT_EQ("a", "a");
}

TEST(FooTest, test_utils_for_time_operation) {
    std::cout << getSystemTimeInMills() << std::endl;
}
/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-07 18:54:27
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-07 19:01:13
 * @FilePath: /lsm-KV-store/test/test_0.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include "../src/utils_for_time_operation.h"
#include <gtest/gtest.h>

TEST(TESTCASE, test0) {
    EXPECT_EQ(0, 0);
    EXPECT_EQ("a", "a");
}

TEST(TESTCASE, test_utils_for_time_operation) {
    std::cout<<getSystemTimeInMills()<<std::endl;
}
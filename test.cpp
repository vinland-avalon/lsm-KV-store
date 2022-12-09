/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-09 16:25:45
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-09 17:53:19
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
using json = nlohmann::json;

TEST(FooTest, test0) {
    EXPECT_EQ(0, 0);
    EXPECT_EQ("a", "a");
}

TEST(UtilsForFileOperation, test){
    EXPECT_EQ(isSolidDirectory("."), true);
    EXPECT_EQ(isSolidDirectory("invalid_dir"), false);
    EXPECT_GT(getFilenamesInDirectory(".").size(), 0);
    EXPECT_EQ(isFileExisting("test.cpp"), true);
    EXPECT_EQ(isFileExisting("invalid_file"), false);
}


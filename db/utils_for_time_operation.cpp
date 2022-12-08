/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-06 17:32:47
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-08 21:11:51
 * @FilePath: /lsm-KV-store/db/utils_for_time_operation.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include <string>
#include <sys/time.h>

std::string getSystemTimeInMills() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return std::to_string(tv.tv_sec) + std::to_string(tv.tv_usec);
}
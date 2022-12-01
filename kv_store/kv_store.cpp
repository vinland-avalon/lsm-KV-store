/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 10:30:34
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-01 01:39:59
 * @FilePath: /lsm-KV-store/kv_store/kv_store.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */
#include <string>

class KvStore {
  public:
    virtual void set(std::string, std::string) = 0;
    virtual std::string get(std::string) = 0;
    virtual void remove(std::string) = 0;
};
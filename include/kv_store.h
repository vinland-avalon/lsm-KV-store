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
#ifndef _KV_STORE_H_
#define _KV_STORE_H_

#include <string>

class KvStore {
  public:
    virtual void Set(std::string, std::string) = 0;
    virtual std::string Get(std::string) = 0;
    virtual void Remove(std::string) = 0;
};

#endif
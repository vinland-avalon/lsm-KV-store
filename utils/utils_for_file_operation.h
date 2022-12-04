/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-03 22:36:58
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-04 11:33:03
 * @FilePath: /lsm-KV-store/utils/utils_for_file_operation.h
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#ifndef _Utils_For_File_Operation_H_
#define _Utils_For_File_Operation_H_

#include <string>
#include <vector>

std::vector<std::string> *getFilenamesInDirectory(const char *dir_name);
bool endsWith(std::string s, std::string sub);
void writeStringToFile(std::string s, std::fstream *f);

#endif
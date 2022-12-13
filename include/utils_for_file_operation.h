/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-03 22:36:58
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-09 17:35:24
 * @FilePath: /lsm-KV-store/include/utils_for_file_operation.h
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#ifndef _Utils_For_File_Operation_H_
#define _Utils_For_File_Operation_H_

#include <string>
#include <vector>

bool IsSolidDirectory(const std::string dir_path);
std::vector<std::string> GetFilenamesInDirectory(const std::string dir_path);
bool EndsWith(std::string s, std::string sub);
void WriteStringToFile(std::string s, std::fstream *f);
bool IsFileExisting(std::string name);
bool OpenFileAndCreateOneWhenNotExist(std::fstream *f, std::string file_path);
bool DeleteFilesInDir(const char *path);


#endif
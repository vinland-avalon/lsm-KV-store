/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-03 22:18:01
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-10 10:49:15
 * @FilePath: /lsm-KV-store/db/utils_for_file_operation.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#include <dirent.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <filesystem>

bool isSolidDirectory(const std::string dir_path) {
    // check if dir_name is a valid dir
    struct stat s;
    lstat(dir_path.c_str(), &s);
    if (!S_ISDIR(s.st_mode)) {
        std::cout << "Warning: [isSolidDIrectory]: " << dir_path << " is not a valid directory !" << std::endl;
        return false;
    }
    return true;
}

/**
 * @description: get filenames in a directory, except for . and ..
 * @param {char} *dir_name
 * @return {*}
 */
std::vector<std::string> getFilenamesInDirectory(const std::string dir_path) {
    std::vector<std::string> filenames;
    for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
        auto filename = entry.path().filename().string();
        // ignore "." and ".."
        if (filename != "." && filename != "..")
            filenames.push_back(std::move(filename));
    }
    return filenames;
}

/**
 * @description:
 * @param {string} s
 * @param {string} sub
 * @return {*}
 */
bool endsWith(std::string s, std::string sub) {
    return s.rfind(sub) == (s.length() - sub.length()) ? true : false;
}

void writeStringToFile(std::string s, std::fstream *f) {
    f->write(s.c_str(), s.size() + 1);
}

bool isFileExisting(std::string name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

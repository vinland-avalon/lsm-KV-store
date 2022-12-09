/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-03 22:18:01
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-09 17:52:23
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
    struct dirent *filename; // return value for readdir()
    DIR *dir;                // return value for opendir()
    dir = opendir(dir_path.c_str());
    if (dir == nullptr) {
        std::cout << "Can not open dir " << dir_path << std::endl;
        return filenames;
    }
    std::cout << "Info: [getFilenamesInDirectory] Successfully opened the dir: " << dir_path << std::endl;
    while ((filename = readdir(dir)) != NULL) {
        // ignore "." and ".."
        if (strcmp(filename->d_name, ".") == 0 || strcmp(filename->d_name, "..") == 0)
            continue;
        // std::cout << filename->d_name << std::endl;
        filenames.push_back(filename->d_name);
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

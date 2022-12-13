/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-03 22:18:01
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-10 16:17:35
 * @FilePath: /lsm-KV-store/db/utils_for_file_operation.cpp
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#include "spdlog/spdlog.h"
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

bool IsSolidDirectory(const std::string dir_path) {
    // check if dir_name is a valid dir
    struct stat s;
    if (lstat(dir_path.c_str(), &s) < 0) {
        spdlog::error("[isSolidDirectory] Failed to lstat {}: {}", dir_path, strerror(errno));
        return false;
    }
    if (!S_ISDIR(s.st_mode)) {
        spdlog::warn("[isSolidDIrectory] {}(dir_path) is not a directory");
        return false;
    }
    return true;
}

/**
 * @description: get filenames in a directory, except for . and ..
 * @param {char} *dir_name
 * @return {*}
 */
std::vector<std::string> GetFilenamesInDirectory(const std::string dir_path) {
    std::vector<std::string> filenames;
    for (const auto &entry : std::filesystem::directory_iterator(dir_path)) {
        auto filename = entry.path().filename().string();
        // ignore "." and ".."
        if (filename != "." && filename != "..")
            filenames.push_back(std::move(filename));
    }
    spdlog::info("[getFilenamesInDirectory] get {0} files in {1}, namely: ", filenames.size(), dir_path);
    for (auto &filename : filenames) {
        spdlog::info("[getFilenamesInDirectory] get {}", filename);
    }
    return filenames;
}

/**
 * @description:
 * @param {string} s
 * @param {string} sub
 * @return {*}
 */
bool EndsWith(std::string s, std::string sub) {
    return s.rfind(sub) == (s.length() - sub.length()) ? true : false;
}

void WriteStringToFile(std::string s, std::fstream *f) {
    f->write(s.c_str(), s.size() + 1);
}

bool IsFileExisting(std::string name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

bool OpenFileAndCreateOneWhenNotExist(std::fstream *f, std::string filePath) {
    f->open(filePath, std::ios::out | std::ios::binary | std::ios::app);
    f->close();
    f->open(filePath, std::ios::out | std::ios::binary | std::ios::in | std::ios::app);
    return true;
}

void Getfilepath(const char *path, const char *filename, char *filePath) {
    strcpy(filePath, path);
    if (filePath[strlen(path) - 1] != '/')
        strcat(filePath, "/");
    strcat(filePath, filename);
}

bool DeleteFilesInDir(const char *path) {
    DIR *dir;
    struct dirent *dirinfo;
    struct stat statbuf;
    char filepath[256] = {0};
    lstat(path, &statbuf);

    // it's a file
    if (S_ISREG(statbuf.st_mode)) {
        remove(path);
    }
    // it's a directory
    else if (S_ISDIR(statbuf.st_mode)) {
        if ((dir = opendir(path)) == NULL)
            return 1;
        while ((dirinfo = readdir(dir)) != NULL) {
            Getfilepath(path, dirinfo->d_name, filepath);
            if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0)
                continue;
            DeleteFilesInDir(filepath);
            rmdir(filepath);
        }
        closedir(dir);
    }
    return 0;
}

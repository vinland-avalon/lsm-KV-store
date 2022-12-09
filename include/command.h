/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-07 12:08:07
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-07 17:24:16
 * @FilePath: /lsm-KV-store/src/command.h
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#ifndef _Command_H_
#define _Command_H_

#include "utils_for_file_operation.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;

// enum enumCommand {"RM","GET","SET"};

class Command {
  public:
    Command(std::string _type, std::string _key) : type(_type), key(_key) {}
    std::string getKey() {
        return this->key;
    };
    virtual json toJSON() = 0;
    /**
     * @description:
     * @param {fstream} *f
     * @return long: the size of serialized command
     */
    long writeCommandToFile(std::fstream *f) {
        std::string commandString = this->toJSON().dump();
        writeStringToFile(commandString, f);
        return commandString.size();
    }
    std::string getType() {
        return this->type;
    }

  protected:
    std::string type;
    std::string key;
};

class SetCommand : public Command {
  public:
    SetCommand(std::string _type, std::string _key, std::string _value) : Command(_type, _key), value(_value) {}
    SetCommand();
    json toJSON() {
        json record;
        record["key"] = key;
        record["type"] = type;
        record["value"] = value;
        return record;
    }
    std::string getValue() {
        return this->value;
    }

  private:
    std::string value;
};

class RmCommand : public Command {
  public:
    RmCommand(std::string _type, std::string _key) : Command(_type, _key) {}
    json toJSON() {
        json record;
        record["key"] = key;
        record["type"] = type;
        return record;
    }
};

#endif
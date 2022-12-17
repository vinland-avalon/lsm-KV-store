/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-12-07 12:08:07
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-10 17:33:44
 * @FilePath: /lsm-KV-store/include/command.h
 * @Description:
 *
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved.
 */

#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "utils_for_file_operation.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;

// enum enumCommand {"RM","GET","SET"};
// todo: instead of JSON to represent a command, use slice and different keys
// for example, in LevelDB:
// key_length, user_key, type, sequence, value_length, value
class Command {
  public:
    Command(std::string _type, std::string _key) : type(_type), key(_key) {}
    std::string GetKey() const {
        return this->key;
    };
    virtual json ToJSON() const = 0;
    /**
     * @description:
     * @param {fstream} *f
     * @return long: the size of serialized command
     */
    long WriteCommandToFile(std::fstream *f) const {
        std::string command_string = this->ToJSON().dump();
        WriteStringToFile(command_string, f);
        return command_string.size();
    }
    std::string GetType() const {
        return this->type;
    }

  protected:
    std::string type;
    std::string key;
};

class RmCommand : public Command {
  public:
    RmCommand(std::string _type, std::string _key) : Command(_type, _key) {}
    json ToJSON() const {
        json record;
        record["key"] = key;
        record["type"] = type;
        return record;
    }
};

class SetCommand : public Command {
  public:
    SetCommand(std::string _type, std::string _key, std::string _value) : Command(_type, _key), value(_value) {}
    json ToJSON() const {
        json record;
        record["key"] = key;
        record["type"] = type;
        record["value"] = value;
        return record;
    }
    std::string GetValue() const {
        return this->value;
    }

  private:
    std::string value;
};

#endif
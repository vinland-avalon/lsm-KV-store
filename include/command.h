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
// todo: batch_command

enum CommandType{
    SetType = 0,
    RemoveType = 1
};
enum CommandFieldJson{
  TypeField = 0,
  KeyField = 1,
  ValueField = 2
};
class Command {
  public:
    static Command* InitSetCommand(std::string key, std::string value){
        auto command = new Command();
        command->type_ = SetType;
        command->key_ = key;
        command->value_ = value;
        command->command_json_[TypeField] = command->GetType();
        command->command_json_[KeyField] = command->GetKey();
        command->command_json_[ValueField] = command->GetValue();
        command->command_string_ = command->command_json_.dump();
        return command;
    }
    static Command* InitRemoveCommand(std::string key){
        auto command = new Command();
        command->type_ = SetType;
        command->key_ = key;
        command->command_json_[TypeField] = command->GetType();
        command->command_json_[KeyField] = command->GetKey();
        command->command_string_ = command->command_json_.dump();
        return command;
    }
    
    json ToJSON() const {
      return command_json_;
    }
    std::string ToString() const {
      return command_string_;
    }
    /**
     * @description:
     * @param {fstream} *f
     * @return long: the size of serialized command
     */
    long WriteCommandToFile(std::fstream *f) const {
        WriteStringToFile(command_string_, f);
        return command_string_.size();
    }
    CommandType GetType() const {
        return type_;
    }
    std::string GetKey() const {
        return key_;
    }
    std::string GetValue() const {
        return value_;
    }

  private:
    CommandType type_;
    std::string key_;
    std::string value_;
    json command_json_;
    std::string command_string_;
};

#endif
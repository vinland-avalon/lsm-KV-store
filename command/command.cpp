/*
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 11:12:10
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-01 01:21:28
 * @FilePath: /lsm-KV-store/command/command.cpp
 * @Description: 
 * 
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved. 
 */
#include <nlohmann/json.hpp>
#include <string>
using json = nlohmann::json;

// enum enumCommand {"RM","GET","SET"};

class Command{
public:
    Command(std::string _type, std::string _key):type(_type), key(_key){}
    std::string getKey(){
        return this->key;
    };
    virtual json toJSON() = 0;
private:
    std::string type;
    std::string key;
};

class setCommand:public Command{
public:
    setCommand(std::string _type, std::string _key, std::string _value):Command(_type, _key), value(_value){}
    setCommand();
    json toJSON(){
        
    }
private:
    std::string value;
};

class rmCommand:public Command{
public:
    rmCommand(std::string _type, std::string _key):Command(_type, _key){}
};
#ifndef _Utils_For_Json_Opeartion_H_
#define _Utils_For_Json_Operation_H_

#include "command.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::shared_ptr<Command> JSONtoCommand(json command_JSON) {
    std::shared_ptr<Command> command;
    if (command_JSON["type"] == "SET") {
        std::shared_ptr<std::string> ss = std::shared_ptr<std::string>(new std::string("AAA"));
        command = std::shared_ptr<Command>(new SetCommand("SET", command_JSON["key"], command_JSON["value"]));
    } else if (command_JSON["type"] == "RM") {
        command = std::shared_ptr<Command>(new RmCommand("RM", command_JSON["key"]));
    } else {
        command = nullptr;
    }
    return command;
    };

#endif
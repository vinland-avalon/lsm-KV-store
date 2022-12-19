#ifndef _Utils_For_Json_Opeartion_H_
#define _Utils_For_Json_Operation_H_

#include "command.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::shared_ptr<Command> JSONtoCommand(json command_JSON) {
    std::shared_ptr<Command> command;
    if (command_JSON[TypeField] == SetType) {
        command = std::shared_ptr<Command>(Command::InitSetCommand(command_JSON[KeyField], command_JSON[ValueField]));
    } else if (command_JSON[TypeField] == RemoveType) {
        command = std::shared_ptr<Command>(Command::InitRemoveCommand(command_JSON[KeyField]));
    } else {
        command = nullptr;
    }
    return command;
};

#endif
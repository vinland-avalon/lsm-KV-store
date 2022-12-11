#ifndef _Utils_For_Time_Json_H_
#define _Utils_For_Time_Json_H_

#include "command.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::shared_ptr<Command> JSONtoCommand(json jCommand);

#endif
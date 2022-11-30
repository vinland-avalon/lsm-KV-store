#include <string>

// enum enumCommand {"RM","GET","SET"};

class Command{
public:
    Command(std::string _type, std::string _key):type(_type), key(_key){}
    std::string getKey(){
        return this->key;
    };
private:
    std::string type;
    std::string key;
};

class setCommand:public Command{
public:
    setCommand(std::string _type, std::string _key, std::string _value):Command(_type, _key), value(_value){}
private:
    std::string value;
};

class rmCommand:public Command{
public:
    rmCommand(std::string _type, std::string _key):Command(_type, _key){}
};
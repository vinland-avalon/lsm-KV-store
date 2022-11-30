#include "./mem_table.cpp"
#include <map>
#include "../command/command.cpp"

class RedBlackTreeMemTable: public MemTable {
public:
    std::string get(std::string key){}
    void set(std::string key, std::string value){}
    void remove(std::string key){}
private:
    std::map<std::string, Command> table;
};
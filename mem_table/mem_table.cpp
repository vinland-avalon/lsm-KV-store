#include <string>

class MemTable {
public:
    virtual std::string get(std::string key) = 0;
    virtual void set(std::string key, std::string value) = 0;
    virtual void remove(std::string key) = 0;
};
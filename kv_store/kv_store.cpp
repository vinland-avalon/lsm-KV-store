#include <string>

class KvStore {
public:
    virtual void set(std::string, std::string) = 0;
    virtual std::string get(std::string) = 0;
    virtual void remove(std::string) = 0;
};
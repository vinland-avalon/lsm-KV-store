#include <fstream>

class TableMetaInfo {
private:
    long version;
    long dataStart;
    long dataLen;
    long indexStart;
    long indexLen;
    long partitionSize;
public:
    void writeToFile(std::fstream* file){}
    static TableMetaInfo readFromFile(std::fstream* file){}
    TableMetaInfo(std::fstream* file){}
    TableMetaInfo(){}
};
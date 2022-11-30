#include <map>
#include <string>
#include <fstream>
#include "./table_meta_info.cpp"
#include "../command/command.cpp"
#include "../mem_table/mem_table.cpp"

//data + sparse index + metainfo
class SsTable {
public:
    SsTable(std::string _filePath, int _partitionSize){}
    static SsTable createFromMemTable(std::string _filePath, int _partitionSize, MemTable* memtable){}
    static SsTable createFromFile(std::string _filePath){}
    Command query(std::string key){}

private:
    TableMetaInfo tableMetaInfo;
    std::map<std::string, std::pair<long, long>> sparseIndex;
    std::fstream file;
    std::string filePath;
};
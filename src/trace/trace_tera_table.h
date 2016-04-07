#pragma once

#include "tera.h"

#include <string>

namespace tera {
class Client;
class Table;
}

namespace baidu {
namespace galaxy {
    namespace trace {

const static std::string LINKBASE_CF = "RAW";
const static std::string LINKBASE_QF = "lb_buf";
const static std::string KVTABLE_CF = "cf";
const static std::string KVTABLE_QF = "v";

class TeraTable {
public:
    explicit TeraTable(const std::string& table_name);
    ~TeraTable();

    static void set_up(const std::string& tera_flag);
    static void tear_down();
    static const std::string& tera_flag();

    tera::Table* table();

private:
    TeraTable();
    TeraTable(const TeraTable&);
    //TeraTable& oprator=(const TeraTable&);

    int open();
    int close();

    static tera::Client* _s_tc;
    static std::string _s_tera_flag;
    std::string _table_name;
    tera::Table* _table;

};

}
}
}

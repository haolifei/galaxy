#include "trace_tera_table.h"
#include <assert.h>
#include <stdlib.h>

#include <glog/logging.h>

namespace baidu {
namespace galaxy {
    namespace trace {

TeraTable::TeraTable(const std::string& table_name) :
    _table_name(table_name),
    _table(NULL) {
    int ret = this->open();

    if (0 != ret) {
        LOG(FATAL) << "open table failure: " << table_name;
        abort();
    }
}

TeraTable::~TeraTable() {
    close();
}

tera::Client* TeraTable::_s_tc = NULL;
std::string TeraTable::_s_tera_flag = "";

void TeraTable::set_up(const std::string& tera_flag) {
    assert(NULL == _s_tc);
    assert(_s_tera_flag.empty());
    assert(!tera_flag.empty());
    tera::ErrorCode ec;
    _s_tc = tera::Client::NewClient(tera_flag, &ec);
    assert(NULL != _s_tc);
    _s_tera_flag = tera_flag;
}

void TeraTable::tear_down() {
    if (NULL != TeraTable::_s_tc) {
        delete TeraTable::_s_tc;
        TeraTable::_s_tc = NULL;
    }
}


const std::string& TeraTable::tera_flag() {
    return _s_tera_flag;
}

tera::Table* TeraTable::table() {
    return _table;
}

int TeraTable::open() {
    tera::ErrorCode ec;
    _table = _s_tc->OpenTable(_table_name, &ec);

    if (NULL == _table) {
        return -1;
    }

    return 0;
}

int TeraTable::close() {
    if (NULL != _table) {
        delete _table;
    }

    return 0;
}

}
}
}

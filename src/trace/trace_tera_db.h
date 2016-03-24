/* 
 * File:   tera_db.h
 * Author: haolifei
 *
 * Created on 2016年3月8日, 下午7:04
 */
#pragma once

#include "trace_db.h"
#include <counter.h>
namespace tera {
    class RowMutation;
}

namespace baidu {
    namespace galaxy {
        namespace trace {
            class TeraTable;
            class TeraDb : public Db {
                public:
                    TeraDb(const std::string& table_name);
                    ~TeraDb();
                    int Open();
                    int Write(boost::shared_ptr<google::protobuf::Message> msg);
                    int Close();
                    int PendingNum();
                    
            private:

                std::string TeraDb::string_value(const google::protobuf::Message* msg,
                                const google::protobuf::Reflection* ref,
                                const google::protobuf::FieldDescriptor* fd);
                static void write_callback(tera::RowMutation* writer);
                struct Context {
                    TeraDb* db;
                };
                
                baidu::common::Counter _pending_num;
                boost::shared_ptr<TeraTable> _tera_table;
                std::string _table_name;
            };
        }
    }
}

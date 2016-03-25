/***************************************************************************
 * 
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/



/**
 * @file src/trace/trace_tera_db.cpp
 * @author haolifei(com@baidu.com)
 * @date 2016/03/09 17:20:20
 * @brief 
 *  
 **/

#include <locale>

#include "trace_tera_db.h"
#include "trace_tera_table.h"
#include "tera.h"
#include "counter.h"

#include <sstream>

namespace baidu {
    namespace galaxy {
        namespace trace {

            typedef union ProtoValue {
                bool bool_value;
                int32_t int32_value;
                int64_t int64_value;
                uint32_t uint32_value;
                uint64_t uint64_value;
                float float_value;
                double double_value;
            };

            TeraDb::TeraDb(const std::string& table_name) :
                _table_name(table_name) {
                
            }

            TeraDb::~TeraDb() {
            }

            int TeraDb::Open() {
                _tera_table.reset(new TeraTable(_table_name));
                return 0;
            }

            int TeraDb::Write(boost::shared_ptr<google::protobuf::Message> msg) {

                const google::protobuf::Reflection* ref = msg->GetReflection();
                const google::protobuf::Descriptor* des = msg->GetDescriptor();

                const google::protobuf::FieldDescriptor* fdes = des->FindFieldByName("row_key");
                if (NULL == fdes) {
                    return -1;
                }

                if (fdes->type() != google::protobuf::FieldDescriptor::TYPE_BYTES &&
                        fdes->type() != google::protobuf::FieldDescriptor::TYPE_STRING) {
                    return -1;
                }
                std::string row_key = ref->GetString(*msg, fdes);
                if (row_key.empty()) {
                    return -1;
                }

                tera::RowMutation* mutation = _tera_table->table()->NewRowMutation(row_key);

                int field_size = des->field_count();
                bool has_data = false;
                for (int i = 0; i < field_size; i++) {
                    const google::protobuf::FieldDescriptor* fd = des->field(i);
                    if (ref->HasField(*msg, fd)) {
                        std::string k = fd->name();
                        if ("row_key" == k) {
                            continue;
                        }
                        std::string v = string_value(msg.get(), ref, fd);
                        std::string c = cf(fd);
                        if (c.empty() || v.empty()) {
                            continue;
                        }

                        has_data = true;
                        mutation->Put(c.c_str(), k, v);
                    }
                }
                
                if (has_data) {
                    mutation->SetCallBack(write_callback);
                    Context* ctx = new Context();
                    ctx->db = this;
                    
                    mutation->SetContext(ctx);
                    _pending_num.Inc();
                    _tera_table->table()->ApplyMutation(mutation);
                }

                return 0;
            }

            int TeraDb::Close() {
                return 0;
            }

            int TeraDb::PendingNum() {
                return _pending_num.Get();
            }

            std::string TeraDb::string_value(const google::protobuf::Message* msg,
                    const google::protobuf::Reflection* ref,
                    const google::protobuf::FieldDescriptor* fd) {
                assert(NULL != fd);
                assert(NULL != msg);
                assert(NULL != ref);

                ProtoValue value;
                std::string str_value;
                std::stringstream stream;

                switch (fd->type()) {
                    case google::protobuf::FieldDescriptor::TYPE_BOOL:
                        value.uint32_value = ref->GetBool(*msg, fd);
                        stream << value.uint32_value;
                        break;
                    case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
                        value.double_value = ref->GetDouble(*msg, fd);
                        stream << value.double_value;
                        break;
                    case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                        value.float_value = ref->GetFloat(*msg, fd);
                        stream << value.float_value;
                        break;
                    case google::protobuf::FieldDescriptor::TYPE_INT32:
                        value.int32_value = ref->GetInt32(*msg, fd);
                        stream << value.int32_value;
                        break;
                    case google::protobuf::FieldDescriptor::TYPE_INT64:
                        value.int64_value = ref->GetInt64(*msg, fd);
                        stream << value.int64_value;
                        break;
                    case google::protobuf::FieldDescriptor::TYPE_STRING:
                    case google::protobuf::FieldDescriptor::TYPE_BYTES:
                        str_value = ref->GetString(*msg, fd);
                        stream << str_value;
                        break;
                    case google::protobuf::FieldDescriptor::TYPE_UINT32:
                        value.uint32_value = ref->GetUInt32(*msg, fd);
                        stream << value.uint32_value;
                        break;
                    case google::protobuf::FieldDescriptor::TYPE_UINT64:
                        value.uint64_value = ref->GetUInt64(*msg, fd);
                        stream << value.uint64_value;
                        break;
                    default:
                        break;
                }

                return stream.str();
            }

            std::stringTeraDb::cf(const google::protobuf::FieldDescriptor* fd) {
                assert(NULL != fd);
                std::string cf = "";
                switch (fd->type()) {
                    case google::protobuf::FieldDescriptor::TYPE_BOOL:
                        cf = "bool";
                        break;
                    case google::protobuf::FieldDescriptor::TYPE_DOUBLE:
                    case google::protobuf::FieldDescriptor::TYPE_FLOAT:
                        cf = "double";
                        break;
                    case google::protobuf::FieldDescriptor::TYPE_INT32:
                    case google::protobuf::FieldDescriptor::TYPE_INT64:
                    case google::protobuf::FieldDescriptor::TYPE_UINT32:
                    case google::protobuf::FieldDescriptor::TYPE_UINT64:
                        cf = "integer";
                        break;
                    case google::protobuf::FieldDescriptor::TYPE_STRING:
                    case google::protobuf::FieldDescriptor::TYPE_BYTES:
                        cf = "string";
                        break;
                    default:
                        break;
                }
                return cf;
            }
            
            void TeraDb::write_callback(tera::RowMutation* writer) {
                assert(NULL != writer);
                Context* con = (Context*)(writer->GetContext());
                assert(NULL != con);
                con->db->_pending_num.Dec();
                delete con;
            }

}
}
}


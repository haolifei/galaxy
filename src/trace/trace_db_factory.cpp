#include "trace_db_factory.h"
#include "trace_mysql_db_pool.h"
#include <assert.h>
#include <gflags/gflags.h>
#include <iostream>

DECLARE_string(mysql_host);
DECLARE_int32(mysql_port);
DECLARE_string(mysql_user);
DECLARE_string(mysql_passwd);
DECLARE_string(mysql_database);
DECLARE_int32(mysql_write_thread_num);

namespace baidu {
    namespace galaxy {
        namespace trace {
            DbFactory* DbFactory::s_instance = NULL;

            DbFactory::DbFactory() {
                assert(NULL == s_instance);
                s_instance = this;
            }

            DbFactory::~DbFactory() {

            }

            int DbFactory::Init() {
                assert(NULL == _mysql.get());
                assert(NULL == _tera_db.get());

                boost::shared_ptr<MysqlDb::Option> op(new MysqlDb::Option());
                op->SetServer(FLAGS_mysql_host)
                    ->SetUser(FLAGS_mysql_user)
                    ->SetPasswd(FLAGS_mysql_passwd)
                    ->SetPort(FLAGS_mysql_port)
                    ->SetDatabase(FLAGS_mysql_database);
                _mysql.reset(new MysqlPool(op, FLAGS_mysql_write_thread_num));

                _tera_db.reset(new TeraDb("galaxy_proc"));

                if (0 != _mysql->Open() || 0 != _tera_db->Open()) {
                    _mysql.reset();
                    _tera_db.reset();
                    return -1;
                } 
                
                _m_db["baidu.galaxy.trace.TracePod"] =  _mysql;
                _m_db["baidu.galaxy.trace.TraceJob"] = _mysql;
                _m_db["baidu.galaxy.trace.TracePodHistory"] = _mysql;
                _m_db["baidu.galaxy.trace.TraceJobMeta"] = _mysql;
                _m_db["baidu.galaxy.trace.TraceJobHistory"] = _mysql;
                _m_db["baidu.galaxy.trace.TraceClusterHistory"] = _mysql;
                _m_db["baidu.galaxy.trace.TraceCluster"] = _mysql;
                _m_db["baidu.galaxy.trace.TraceAgentError"] = _mysql;
                _m_db["baidu.galaxy.trace.TraceAgent"] = _mysql;

                _m_db["baidu.galaxy.trace.TracePodMetrix"] = _tera_db;
                _m_db["baidu.galaxy.trace.TraceAgentMetrix"] = _tera_db;
                _m_db["baidu.galaxy.trace.TraceJobMerix"] = _tera_db;
                _m_db["baidu.galaxy.trace.TraceAgent"] = _tera_db;
                return 0;
            }

            boost::shared_ptr<Db> DbFactory::GetDb(boost::shared_ptr<google::protobuf::Message> msg) {
                boost::shared_ptr<Db> ret;
                std::map<std::string, boost::shared_ptr<Db> >::iterator iter = _m_db.find(msg->GetDescriptor()->full_name());
                if (_m_db.end() != iter) {
                    ret = iter->second;
                }
                
                return ret;
            }
            
        }
    }
}

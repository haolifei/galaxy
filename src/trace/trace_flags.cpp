/***************************************************************************
 * 
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file trace_flags.cc
 * @author haolifei(com@baidu.com)
 * @date 2016/04/07 14:21:05
 * @brief 
 *  
 **/


#include <gflags/gflags.h>

DEFINE_string(trace_server_address, "0.0.0.0:8888", "trace server address");
DEFINE_string(tera_flag, "", "tera flag");
DEFINE_string(mysql_host, "", "-h");
DEFINE_int32(mysql_port, 0, "-P");
DEFINE_string(mysql_user, "", "-u");
DEFINE_string(mysql_passwd, "lumia123!@#", "-p");
DEFINE_string(mysql_database, "", "data base");
DEFINE_int32(mysql_write_thread_num, 10, "");

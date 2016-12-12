/***************************************************************************
 * 
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file ../example/test_naming.cc
 * @author haolifei(com@baidu.com)
 * @date 2016/12/12 15:52:10
 * @brief 
 *  
 **/

#include <iostream>
#include "appmaster_v2/naming/naming_manager.h"
#include "gflags/gflags.h"
#include "protocol/galaxy.pb.h"
#include "appmaster_v2/namespace.h"

namespace am = baidu::galaxy::am;


int main(int argc, char** argv) {
    //
    if (argc != 4) {
        std::cerr << argc << std::endl;
        std::cerr << "usage: " << argv[0] << " node token --flagfile=file_path\n";
        return -1;
    }

    google::ParseCommandLineFlags(&argc, &argv, true);
    std::string name = argv[1];
    std::string token = argv[2];

    const std::string tag = "tag";
    const std::string job = "test_job";

    am::NamingManager ns;
    baidu::galaxy::util::ErrorCode ec = ns.RegisterService(name, token, tag, job);
    if (ec.Code() != 0) {
        std::cerr << "register " << name << " failed: " << ec.Message();
        return -1;
    }

    int n = 10;
    while (true) {
        if (n) {
            boost::shared_ptr<am::NamingInstance> node(new am::NamingInstance);
            proto::ServiceInfo si;
            si.set_name(name);
            si.set_hostname("yq01-spi-galaxy508.yq01");
            si.set_port("1025");
            si.set_deploy_path("/home/galaxy/");

            char buf[32] = {0};
            snprintf(buf, sizeof buf, "pod_%d_0", n);
            si.set_task_id(buf);
            node->FromProto(si);
            std::list<boost::shared_ptr<am::NamingInstance> > l;
            l.push_back(node);
            ec = ns.SyncInstances(name, l);
            if (ec.Code() != 0) {
                std::cerr << "update instance failed: " << ec.Message() << std::endl;
            }

           // n--;
        }
        sleep(1);
        std::cerr << "=========\n"; 
    }

    return 0;
}


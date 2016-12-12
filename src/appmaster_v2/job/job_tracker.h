// Copyright (c) 2016, Baidu.com, Inc. All Rights Reserved
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once
#include "job.h"
#include "thread_pool.h"
#include "protocol/galaxy.pb.h"
#include "protocol/appmaster.pb.h"
#include "agent/util/error_code.h"

#include <list>
#include <map>

namespace baidu {
namespace galaxy {
namespace am {

class JobTracker {
public:
    class Counter {
    public:
        int pending_num;
        int death_num;
        int failed_num;
        int running_num;
        int finished_updating_num;
        int deploying_num;

    public:
        Counter() :
            pending_num(0),
            death_num(0),
            failed_num(0),
            running_num(0),
            finished_updating_num(0),
            deploying_num(0) {}
    };

public:
    JobTracker(const JobId& jobid, const baidu::galaxy::proto::JobDescription& desc);
    ~JobTracker();

    void Run(boost::shared_ptr<baidu::common::ThreadPool> running_pool);

    baidu::galaxy::util::ErrorCode Update(const baidu::galaxy::proto::JobDescription& desc,
            int breakpoint,
            bool res_changed);
    baidu::galaxy::util::ErrorCode ContinueUpdating(int breakpoint);
    baidu::galaxy::util::ErrorCode PauseUpdating();
    baidu::galaxy::util::ErrorCode RollbackUpdating();
    baidu::galaxy::util::ErrorCode CancelUpdating();

    baidu::galaxy::util::ErrorCode ForceAction(const PodId& id,
            baidu::galaxy::proto::UpdateAction action);

    baidu::galaxy::util::ErrorCode ForceCommand(const PodId& id,
            const std::string& action);

    baidu::galaxy::util::ErrorCode HandleFetch(const baidu::galaxy::proto::FetchTaskRequest& request,
            baidu::galaxy::proto::FetchTaskResponse& response);

    void ListServices(std::map<std::string, proto::Service>& services);
    baidu::galaxy::util::ErrorCode ListServiceInfo(const std::string& service_name, std::vector<proto::ServiceInfo>& info);

    void Destroy();
    void TearDown();

    // read
    const JobId& Id();
    const proto::JobStatus Status();
    const proto::JobDescription& Description();

    bool LastDescriptionCopy(proto::JobDescription& desc);
    bool DescriptionCopy(proto::JobDescription& desc);

    void GetCounter(Counter& cnt);
    JobVersion Version();
    JobVersion LastVersion();
    void PodInfo(::google::protobuf::RepeatedPtrField<proto::PodInfo >* pods);
    size_t PodSize();
    const std::string DebugString();

private:
    baidu::galaxy::util::ErrorCode CheckJobDescription(const baidu::galaxy::proto::JobDescription& desc);

    void ExportPodDescription(const baidu::galaxy::proto::JobDescription& job,
            baidu::galaxy::proto::PodDescription& pod);

    void ExportContainerDescription(const baidu::galaxy::proto::JobDescription& job,
            baidu::galaxy::proto::ContainerDescription& container);

    void CheckDeadPodLoop(int32_t interval);
    void CheckDeployLoop(int32_t interval);

    void ChangeStatusTo(proto::JobStatus status, const std::string& f, const int line);

    const JobId id_;
    boost::shared_ptr<baidu::galaxy::proto::JobDescription> desc_;
    boost::shared_ptr<baidu::galaxy::proto::JobDescription> last_desc_;

    JobVersion version_;
    JobVersion last_version_;
    baidu::galaxy::proto::JobStatus status_;
    proto::UpdateAction update_action_;
    int breakpoint_;

    std::list<boost::shared_ptr<RuntimePod> > runtime_pods_;
    std::map<PodId, boost::shared_ptr<RuntimePod> > runtime_pods_index_;
    std::map<PodId, boost::shared_ptr<RuntimePod> > recreating_pods_index_;
    std::map<PodId, boost::shared_ptr<RuntimePod> > reloading_pods_index_;

    boost::shared_ptr<baidu::common::ThreadPool> running_pool_;
    boost::mutex mutex_;
    bool running_;
    int64_t check_dead_thread_;
    int64_t check_deploy_thread_;
};

}
}
}

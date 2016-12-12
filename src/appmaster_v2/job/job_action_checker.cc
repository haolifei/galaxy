/***************************************************************************
 *
 * Copyright (c) 2016 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file job/job_action_checker.cc
 * @author haolifei(com@baidu.com)
 * @date 2016/11/29 17:22:40
 * @brief
 *
 **/

#include "job_action_checker.h"
#include "glog/logging.h"

namespace baidu {
namespace galaxy {
namespace am {

JobActionChecker::JobActionChecker() {
}


JobActionChecker::~JobActionChecker() {
}

void JobActionChecker::Setup() {
    pending_disallowed_action_.insert(proto::kUpdateContinue);
    pending_disallowed_action_.insert(proto::kUpdateRollback);
    pending_disallowed_action_.insert(proto::kUpdateCancel);
    pending_disallowed_action_.insert(proto::kPauseUpdate);
    disallowed_action_map_[proto::kJobPending] = &pending_disallowed_action_;
    running_disallowed_action_.insert(proto::kUpdateContinue);
    running_disallowed_action_.insert(proto::kUpdateRollback);
    running_disallowed_action_.insert(proto::kUpdateCancel);
    running_disallowed_action_.insert(proto::kPauseUpdate);
    disallowed_action_map_[proto::kJobRunning] = &running_disallowed_action_;
    finished_disallowed_action_.insert(proto::kUpdate);
    finished_disallowed_action_.insert(proto::kUpdateContinue);
    finished_disallowed_action_.insert(proto::kUpdateRollback);
    finished_disallowed_action_.insert(proto::kUpdateCancel);
    finished_disallowed_action_.insert(proto::kPauseUpdate);
    disallowed_action_map_[proto::kJobFinished] = &finished_disallowed_action_;
    destroy_disallowed_action_.insert(proto::kUpdate);
    destroy_disallowed_action_.insert(proto::kUpdateContinue);
    destroy_disallowed_action_.insert(proto::kUpdateRollback);
    destroy_disallowed_action_.insert(proto::kUpdateCancel);
    destroy_disallowed_action_.insert(proto::kPauseUpdate);
    disallowed_action_map_[proto::kJobDestroying] = &destroy_disallowed_action_;
    updating_disallowed_action_.insert(proto::kUpdate);
    updating_disallowed_action_.insert(proto::kUpdateContinue);
    //updating_disallowed_action_.insert(proto::kUpdateRollback);
    disallowed_action_map_[proto::kJobUpdating] = &updating_disallowed_action_;
    updatapause_disallowed_action_.insert(proto::kUpdate);
    updatapause_disallowed_action_.insert(proto::kPauseUpdate);
    disallowed_action_map_[proto::kJobUpdatePause] = &updatapause_disallowed_action_;
}

baidu::galaxy::util::ErrorCode
JobActionChecker::CheckAction(proto::JobStatus status, proto::JobEvent action) const {
    std::map<proto::JobStatus, std::set<proto::JobEvent>* >::const_iterator iter = disallowed_action_map_.find(status);

    if (iter == disallowed_action_map_.end()) {
        LOG(FATAL) << "status donot exist: " << proto::JobStatus_Name(status) << " " << status;
    }

    if (iter->second->find(action) != iter->second->end()) {
        return ERRORCODE(-1, "%s is conflict with %s",
                proto::JobStatus_Name(status).c_str(),
                proto::JobEvent_Name(action).c_str());
    }

    return ERRORCODE_OK;
}

}
}
}





















/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */

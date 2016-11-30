#pragma once

#include "agent/util/error_code.h"
#include "protocol/galaxy.pb.h"

#include <map>
#include <set>

namespace proto = baidu::galaxy::proto;

namespace baidu {
    namespace galaxy {
        namespace am {

            class JobActionChecker {
                public:
                    JobActionChecker();
                    ~JobActionChecker();
                    void Setup();
                    baidu::galaxy::util::ErrorCode CheckAction(proto::JobStatus status, proto::JobEvent) const;

                private:
                    std::set<proto::JobEvent> pending_disallowed_action_;
                    std::set<proto::JobEvent> running_disallowed_action_;
                    std::set<proto::JobEvent> finished_disallowed_action_;
                    std::set<proto::JobEvent> destroy_disallowed_action_;
                    std::set<proto::JobEvent> updating_disallowed_action_;
                    std::set<proto::JobEvent> updatapause_disallowed_action_;

                    std::map<proto::JobStatus, std::set<proto::JobEvent>* > disallowed_action_map_;
            };
        }
    }
}

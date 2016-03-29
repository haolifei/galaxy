#include "trace_galaxy.h"
#include "proto/trace_log.pb.h"
#include "master/job_manager.h"
#include "agent/agent_internal_infos.h"
#include "trace_client/trace_sdk.h"
#include "master/job_manager.h"
#include "proto/master.pb.h"
#include "trace_client/trace_util.h"

#include <gflags/gflags.h>

#include <logger.h>
DECLARE_string(agent_ip);
DECLARE_string(agent_port);
DECLARE_string(nexus_root_path);

namespace baidu {
    namespace galaxy {
        namespace trace {

            GalaxyAgentTracer::GalaxyAgentTracer() {

            }

            GalaxyAgentTracer* GalaxyAgentTracer::s_instance = NULL;

            GalaxyAgentTracer* GalaxyAgentTracer::GetInstance() {
                if (NULL != s_instance) {
                    s_instance = new GalaxyAgentTracer();
                }
                return s_instance;
            }

            int GalaxyAgentTracer::Setup() {
                std::string trace_server = "10.86.58.29:8888";
                if (0 != baidu::galaxy::trace::Trace::GetInstance()->Setup(trace_server)) {
                    LOG(WARNING, "set up trace server failed");
                    return -1;
                }
                LOG(INFO, "set up trace successfully");
                return 0;
            }

            int GalaxyAgentTracer::TearDown() {
                return 0;
            }

            int GalaxyAgentTracer::TraceTaskEvent(const TaskInfo* task,
                    const std::string& event,
                    bool inner_error,
                    int code) {
                baidu::galaxy::trace::TracePodHistory tph;

                tph.set_time(string_time(baidu::common::timer::now_time()));
                tph.set_event(event);
                tph.set_endpoint(FLAGS_agent_ip);
                tph.set_errcode(code);
                tph.set_reason(event);
                tph.set_gc_dir(task->gc_dir);
                tph.set_pod(task->pod_id);
                tph.set_job(task->job_id);
                baidu::galaxy::trace::Trace::GetInstance()->Log(&tph);
                return 0;
            }

            int GalaxyAgentTracer::TracePodStatus(const PodStatus* pod,
                    int32_t cpu_quota,
                    int64_t mem_quota) {
                const static std::string endpoint = FLAGS_agent_ip + ":" + FLAGS_agent_port;
                baidu::galaxy::trace::TracePod tp;
                tp.set_id(pod->podid());
                tp.set_state((int) pod->state());
                tp.set_endpoint(endpoint);
                tp.set_version(pod->version());
                tp.set_cpu_assigned(cpu_quota);
                tp.set_cpu_used(pod->resource_used().millicores());
                tp.set_memory_assigned(mem_quota);
                tp.set_memory_used(pod->resource_used().memory());
                tp.set_disk_read(pod->resource_used().syscr_ps());
                tp.set_disk_write(pod->resource_used().syscw_ps());
                tp.set_job(pod->jobid());
                baidu::galaxy::trace::Trace::GetInstance()->Log(&tp);
                return 0;
            }

            int GalaxyAgentTracer::TracePodStart(const std::string& job, const std::string& pod) {
                baidu::galaxy::trace::TracePod tp;
                tp.set_id(pod);
                tp.set_job(job);
                //tp.set_start_time(baidu::common::timer::now_time());
                return 0;
            }

            int GalaxyAgentTracer::TracePodFinished(const std::string& pod, int errcode) {
                baidu::galaxy::trace::TracePod tp;
                tp.set_id(pod);
                //tp.set_finished_time(baidu::common::timer::now_time());
                //tp.set_err_code(errcode);
                return 0;
            }

            int GalaxyAgentTracer::TraceAgentStatus(const AgentInfo* ai) {
                baidu::galaxy::trace::TraceAgentMetrix tam;
                tam.set_key(FLAGS_agent_ip);
                tam.set_time(baidu::common::timer::now_time());
                tam.set_cpu_assgned(ai->assigned().millicores());
                tam.set_cpu_used(ai->used().millicores());
                tam.set_memory_assgned(ai->assigned().memory());
                tam.set_memory_assgned(ai->assigned().memory());
                tam.set_disk_read(ai->used().read_bytes_ps());
                tam.set_disk_write(ai->used().write_bytes_ps());
                tam.set_disk_write_qps(ai->used().write_io_ps());
                tam.set_disk_read_qps(ai->used().read_io_ps());
                tam.set_pod_num(ai->pods_size());
                return baidu::galaxy::trace::Trace::GetInstance()->Log(&tam);
            }

            GalaxyMasterTracer::GalaxyMasterTracer() {

            }

            GalaxyMasterTracer* GalaxyMasterTracer::s_instance = NULL;

            GalaxyMasterTracer* GalaxyMasterTracer::GetInstance() {
                if (NULL != s_instance) {
                    s_instance = new GalaxyMasterTracer();
                }
                return s_instance;
            }

            int GalaxyMasterTracer::Setup() {
                std::string trace_server = "10.86.58.29:8888";
                if (0 != baidu::galaxy::trace::Trace::GetInstance()->Setup(trace_server)) {
                    LOG(WARNING, "set up trace server failed");
                    return -1;
                }
                LOG(INFO, "set up trace successfully");
                return 0;
            }

            int GalaxyMasterTracer::TearDown() {
                return 0;
            }

            int GalaxyMasterTracer::TraceJobEvent(const std::string& job,
                        const std::string& event) {
                baidu::galaxy::trace::TraceJobHistory tjh;
                tjh.set_job(job);
                tjh.set_time(string_time(baidu::common::timer::now_time()));
                tjh.set_event(event);
                tjh.set_reason(event);
                baidu::galaxy::trace::Trace::GetInstance()->Log(&tjh);
                return 0;
            }

            int GalaxyMasterTracer::TraceJob(const Job* job) {
                assert(NULL != job);
                baidu::galaxy::trace::TraceJob tj;
                tj.set_id(job->id_);
                tj.set_name(job->desc_.name());
                tj.set_cluster(FLAGS_nexus_root_path);
                tj.set_state(job->state_);

                int32_t deploying = 0;
                int32_t running = 0;
                int32_t pending = 0;
                //int32_t death = 0;
                int64_t cpu_used = 0;
                int64_t mem_used = 0;
                int64_t disk_read = 0;
                int64_t disk_write = 0;
                int64_t term = 0;
                
                std::map<PodId, PodStatus*>::const_iterator it = job->pods_.begin();
                for (; it != job->pods_.end(); ++it) {
                    PodStatus* pod = it->second;
                    cpu_used += pod->resource_used().millicores();
                    mem_used += pod->resource_used().memory();
                    disk_read += pod->resource_used().read_bytes_ps();
                    disk_write += pod->resource_used().write_bytes_ps();
                    
                    if (pod->stage() == kStageRunning) {
                        ++running;
                    } else if (pod->stage() == kStagePending) {
                        ++pending;
                    } else if (pod->state() == kPodDeploying){
                        ++deploying;
                    } else if (pod->state() == kPodTerminate || pod->state() == kPodFinish){
                        ++term;
                    } else {
                        ++term;
                    }
                }
                tj.set_pod_running(running);
                tj.set_pod_deploying(deploying);
                tj.set_pod_pending(pending);
                tj.set_pod_expired(term);
                tj.set_pod_total(job->pods_.size());
                tj.set_cpu(cpu_used);
                tj.set_memory(mem_used);
                tj.set_disk_read(disk_read);
                tj.set_disk_write(disk_write);
                tj.set_created(string_time(job->create_time/1000000));
                tj.set_updated(string_time(job->update_time/1000000));
                tj.set_user("galaxy");

                return baidu::galaxy::trace::Trace::GetInstance()->Log(&tj);
                
            }
          
            int GalaxyMasterTracer::TraceJobStop(const std::string& id, int code) {
                baidu::galaxy::trace::TraceJob tj;
                tj.set_id(id);
                tj.set_finished(string_time(baidu::common::timer::now_time()));
                tj.set_deleted(1);
                return baidu::galaxy::trace::Trace::GetInstance()->Log(&tj);
            }

            int GalaxyMasterTracer::TraceAgentEvent(const AgentInfo* ai, 
                        const std::string& event, 
                        int code) {
                baidu::galaxy::trace::TraceAgentError tae;
                tae.set_hostname(ai->endpoint());
                //tae.set_errno(0);
                tae.set_time(string_time(baidu::common::timer::now_time()));
                tae.set_agent(ai->endpoint());
                tae.set_cluster(FLAGS_nexus_root_path);
                tae.set_reason(event);
                //tae.set_deleted(0);
                baidu::galaxy::trace::Trace::GetInstance()->Log(&tae);
                return 0;
            }

            int GalaxyMasterTracer::TraceAgentStatus(const AgentInfo* ai) {
                assert(NULL != ai);
                baidu::galaxy::trace::TraceAgent ta;
                ta.set_id(ai->endpoint());
                ta.set_hostname(ai->endpoint());
                ta.set_cluster(FLAGS_nexus_root_path);
                ta.set_state((int) ai->state());
                ta.set_pod_total(ai->pods_size());
                ta.set_cpu_total(ai->total().millicores());
                ta.set_cpu_assigned(ai->assigned().millicores());
                ta.set_cpu_used(ai->used().millicores());

                ta.set_memory_assigned(ai->assigned().memory());
                ta.set_memory_total(ai->total().memory());
                ta.set_memory_used(ai->used().memory());

                std::string tags = "";
                int size = ai->tags().size();
                for (int i = 0; i < size; i++) {
                    tags += ai->tags(i);
                }
                ta.set_labels(tags);
                baidu::galaxy::trace::Trace::GetInstance()->Log(&ta);
                return 0;
            }

            int GalaxyMasterTracer::TraceCluster(const baidu::galaxy::trace::TraceCluster& cluster) {
                return baidu::galaxy::trace::Trace::GetInstance()->Log(&cluster);
            }

            int GalaxyMasterTracer::TraceJobMeta(const std::string& job, 
                        const baidu::galaxy::JobDescriptor* job_desc) {
                baidu::galaxy::trace::TraceJobMeta tjm;
                tjm.set_job(job);
                tjm.set_meta(job_desc->DebugString());
                return baidu::galaxy::trace::Trace::GetInstance()->Log(&tjm);
            }

        }
    }
}

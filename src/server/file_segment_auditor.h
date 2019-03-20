//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_FILESEGMENTAUDITOR_H
#define HFETCH_FILESEGMENTAUDITOR_H


#include <src/common/distributed_ds/hashmap/DistributedHashMap.h>
#include <src/common/distributed_ds/map/DistributedMap.h>
#include <src/common/distributed_ds/multimap/DistributedMultiMap.h>
#include <src/common/configuration_manager.h>
#include <src/common/singleton.h>
#include <src/common/macros.h>
#include <src/common/data_structure.h>

class FileSegmentAuditor {
    /* filename to offset_map pointer*/
    typedef DistributedMap<Segment,std::pair<PosixFile,SegmentScore>> SegmentMap;
    std::unordered_map<CharStruct,SegmentMap*> file_segment_map;
    DistributedHashMap<CharStruct,uint32_t> file_active_status;
    std::shared_ptr<RPC> rpc;
    const std::string FILE_SEGMENT_AUDITOR="FILE_SEGMENT_AUDITOR";
    ServerStatus CreateOffsetMap(Event event){
        MPI_Barrier(CONF->server_comm);
        auto file_iter = file_segment_map.find(event.filename);
        if(file_iter == file_segment_map.end()){
            std::string name(event.filename.c_str());
            SegmentMap* mapLayer = new SegmentMap(name + "_SEGEMENT",CONF->is_server,CONF->my_server,CONF->num_servers);
            if(CONF->my_rank_server == 0){
                SegmentScore score;
                score.frequency=0;
                score.time=0;
                PosixFile file;
                file.filename = event.filename;
                file.segment = event.segment;
                file.layer = Layer(event.layer_index);
                mapLayer->Put(event.segment,std::pair<PosixFile,SegmentScore>(file,score));
            }
        }
        MPI_Barrier(CONF->server_comm);
        return ServerStatus::SERVER_SUCCESS;
    }
    ServerStatus SyncCreateOffsetMap(Event event){
        for(int i=0;i<CONF->num_servers;++i){
            if(i!=CONF->my_server){
                rpc->call(i,FILE_SEGMENT_AUDITOR+"_CreateOffsetMap",event).template as<ServerStatus>();
            }else{
                CreateOffsetMap(event);
            }
        }
        return ServerStatus::SERVER_SUCCESS;
    }

    ServerStatus MarkFileSegmentsActive(Event event);

    ServerStatus MarkFileSegmentsInactive(Event event);

    ServerStatus IncreaseFileSegmentFrequency(Event event);
public:
    FileSegmentAuditor():file_segment_map(),file_active_status("FILE_ACTIVE_STATUS",CONF->is_server,CONF->my_server,CONF->num_servers){
        rpc=Singleton<RPC>::GetInstance("RPC_SERVER_LIST",CONF->is_server,CONF->my_server,CONF->comm_size);
        if(CONF->is_server){
            std::function<ServerStatus(Event)> createOffsetMapFunc(std::bind(&FileSegmentAuditor::CreateOffsetMap, this, std::placeholders::_1));
            rpc->bind(FILE_SEGMENT_AUDITOR+"_CreateOffsetMap", createOffsetMapFunc);
        }
    }
    ServerStatus Update(std::vector<Event> events);

    ServerStatus UpdateOnPrefetch(PosixFile source,PosixFile destination);

};


#endif //HFETCH_FILESEGMENTAUDITOR_H

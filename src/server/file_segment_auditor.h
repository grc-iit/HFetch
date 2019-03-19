//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_FILESEGMENTAUDITOR_H
#define HFETCH_FILESEGMENTAUDITOR_H


#include <src/common/distributed_ds/hashmap/DistributedHashMap.h>
#include <src/common/distributed_ds/map/DistributedMap.h>
#include <src/common/configuration_manager.h>
#include <src/common/singleton.h>
#include <src/common/macros.h>
#include <src/common/data_structure.h>

class FileSegmentAuditor {
    /* filename to offset_map pointer*/
    DistributedHashMap<CharStruct,DistributedMap<Segment,SegmentScore>*> file_score_map;
    DistributedHashMap<CharStruct,DistributedMap<Segment,uint8_t>*> file_placement_map;



    std::shared_ptr<RPC> rpc;
    const std::string FILE_SEGMENT_AUDITOR="FILE_SEGMENT_AUDITOR";
    ServerStatus CreateOffsetMap(CharStruct filename){
        MPI_Barrier(CONF->server_comm);
        /* Do the work */
        MPI_Barrier(CONF->server_comm);
        return ServerStatus::SERVER_SUCCESS;
    }
    ServerStatus SyncCreateOffsetMap(CharStruct filename){
        for(int i=0;i<CONF->num_servers;++i){
            if(i!=CONF->my_server){
                rpc->call(i,FILE_SEGMENT_AUDITOR+"_CreateOffsetMap",filename).template as<ServerStatus>();
            }else{
                CreateOffsetMap(filename);
            }
        }
        return ServerStatus::SERVER_SUCCESS;
    }
public:
    FileSegmentAuditor():file_score_map("FILE_SCORE_MAP",CONF->is_server,CONF->my_server,CONF->num_servers),
                         file_placement_map("FILE_SCORE_MAP",CONF->is_server,CONF->my_server,CONF->num_servers){
        rpc=Singleton<RPC>::GetInstance("RPC_SERVER_LIST",CONF->is_server,CONF->my_rank,CONF->comm_size);
        if(CONF->is_server){
            std::function<ServerStatus(CharStruct)> createOffsetMapFunc(std::bind(&FileSegmentAuditor::CreateOffsetMap, this, std::placeholders::_1));
            rpc->bind(FILE_SEGMENT_AUDITOR+"_CreateOffsetMap", createOffsetMapFunc);
        }
    }

    ServerStatus OnOpen(CharStruct filename,Segment segment);
    ServerStatus OnClose(CharStruct filename,Segment segment);
    ServerStatus OnRead(CharStruct filename,Segment segment);
    ServerStatus OnPrefetch(CharStruct filename,Segment segment, uint8_t layer_index);
    ServerStatus GetPrefetchData(CharStruct filename,Segment segment);

};


#endif //HFETCH_FILESEGMENTAUDITOR_H

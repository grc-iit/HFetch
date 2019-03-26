//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_CONFIGURATION_MANAGER_H
#define HFETCH_CONFIGURATION_MANAGER_H


#include <cstdio>
#include <mpi.h>
#include "data_structure.h"
#include "debug.h"

class ConfigurationManager {
public:
    int num_servers,my_rank_world,comm_size,my_rank_server;
    MPI_Comm server_comm;
    bool is_server;
    int ranks_per_server,num_workers,comm_threads_per_server;
    uint16_t my_server;
    DataPlacementEngineType dpeType;
    double hit,total;
    int max_num_files;


    ConfigurationManager():
    num_servers(1),is_server(false),ranks_per_server(1),comm_threads_per_server(1),my_server(0),my_rank_server(0),num_workers(1),
    dpeType(DataPlacementEngineType::MAX_BW),server_comm(),hit(0.0),total(0.0),max_num_files(1){
        AutoTrace trace = AutoTrace("ConfigurationManager");
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank_world);
        max_num_files=comm_size;
    }
    void UpdateServerComm(){
        AutoTrace trace = AutoTrace("ConfigurationManager::UpdateServerComm");
        MPI_Comm_split(MPI_COMM_WORLD, is_server, my_rank_world, &server_comm);
        if(is_server) MPI_Comm_rank(server_comm, &my_rank_server);
    }
    void BuildLayers(LayerInfo* layers,size_t count){
        AutoTrace trace = AutoTrace("ConfigurationManager::BuildLayers",count);
        Layer* current_layer=NULL;
        Layer* previous_layer=NULL;
        for(int order=0;order<count;order++){
            current_layer=new Layer();
            if(order==0){
                Layer::FIRST=current_layer;
            }
            current_layer->id_=order+1;
            current_layer->capacity_mb_=layers[order].capacity_mb_;
            current_layer->io_client_type=layers[order].is_memory?IOClientType::SIMPLE_MEMORY:layers[order].is_local?IOClientType::LOCAL_POSIX_FILE:IOClientType::SHARED_POSIX_FILE;
            current_layer->direct_io=layers[order].direct_io;
            current_layer->bandwidth_mbps_=layers[order].bandwidth;
            strcpy(current_layer->layer_loc.data(),layers[order].mount_point_);
            current_layer->previous=previous_layer == NULL?nullptr:previous_layer;
            current_layer->next= nullptr;
            if(previous_layer != NULL){
                previous_layer->next=current_layer;
            }
            previous_layer=current_layer;
        }
        //delete(layers);
        Layer::LAST=previous_layer;
    }
};


#endif //HFETCH_CONFIGURATION_MANAGER_H

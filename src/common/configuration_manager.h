//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_CONFIGURATION_MANAGER_H
#define HFETCH_CONFIGURATION_MANAGER_H


#include <cstdio>

class ConfigurationManager {
public:
    int num_servers,my_rank_world,comm_size,my_rank_server;
    MPI_Comm server_comm;
    bool is_server;
    size_t ranks_per_server;
    uint16_t my_server;
    DataPlacementEngineType dpeType;

    ConfigurationManager():
    num_servers(1),is_server(false),ranks_per_server(1),my_server(0),my_rank_server(0),
    dpeType(DataPlacementEngineType::MAX_BW),server_comm(){
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank_world);
        MPI_Comm_split(MPI_COMM_WORLD, is_server, my_rank_world, &server_comm);
        MPI_Comm_rank(server_comm, &my_rank_server);
    }
};


#endif //HFETCH_CONFIGURATION_MANAGER_H

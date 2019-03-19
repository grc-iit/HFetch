//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_CONFIGURATION_MANAGER_H
#define HFETCH_CONFIGURATION_MANAGER_H


#include <cstdio>

class ConfigurationManager {
public:
    int num_servers,my_rank,comm_size;
    MPI_Comm server_comm;
    bool is_server;
    size_t ranks_per_server;
    uint16_t my_server;

    ConfigurationManager():num_servers(1),is_server(false),ranks_per_server(1),my_server(0){
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        MPI_Comm_split(MPI_COMM_WORLD, is_server, my_rank, &server_comm);
    }
};


#endif //HFETCH_CONFIGURATION_MANAGER_H

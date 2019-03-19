//
// Created by hariharan on 2/20/19.
//

#ifndef HERMES_PROJECT_RPC_LIB_H
#define HERMES_PROJECT_RPC_LIB_H


#include <cstdint>
#include <rpc/server.h>
#include <mpi.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <rpc/client.h>

class RPC {
private:
    bool isInitialized,is_server;
    int my_rank,comm_size,num_servers;
    uint16_t server_port, my_server;
    std::shared_ptr<rpc::server> server;
    std::vector<std::string> server_list;
public:
    ~RPC(){
    }
    RPC(uint16_t port,bool is_server_,uint16_t my_server_,int num_servers_):num_servers(num_servers_),server_port(port),my_server(my_server_),isInitialized(false),is_server(is_server_),server_list(){
        if(!isInitialized){
            int total_len;
            char *final_server_list;
            /* Initialize MPI rank and size of world */
            MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
            MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
            /* get which server the rank will connect to*/
            /* Create a server communication group */
            MPI_Comm scomm;
            MPI_Comm_split(MPI_COMM_WORLD, is_server, my_rank, &scomm);
            /* Calculate Max num of servers */
            if (is_server) {
                /* Get hostname where server is running Name */
                int len;
                char processor_name[MPI_MAX_PROCESSOR_NAME];
                MPI_Get_processor_name(processor_name, &len);
                /* Get current servers rank in the server group starts with 1*/
                int server_rank = (my_rank % num_servers) + 1;
                /* Synchronize hostnames accross all servers*/
                int *recvcounts = NULL;
                if (server_rank == 1) recvcounts = static_cast<int *>(malloc(num_servers * sizeof(int)));
                MPI_Gather(&len, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, scomm);
                total_len = 0;
                int *displs = NULL;
                char *totalstring = NULL;

                /* if it is the first server*/
                if (server_rank == 1) {
                    displs = static_cast<int *>(malloc(num_servers * sizeof(int)));
                    displs[0] = 0;
                    total_len += recvcounts[0] + 1;
                    for (int i = 1; i < num_servers; i++) {
                        total_len += recvcounts[i] + 1;   /* plus one for space or \0 after words */
                        displs[i] = displs[i - 1] + recvcounts[i - 1] + 1;
                    }
                    /* allocate string, pre-fill with spaces and null terminator */
                    totalstring = static_cast<char *>(malloc(total_len * sizeof(char)));
                    for (int i = 0; i < total_len - 1; i++)
                        totalstring[i] = ',';
                    totalstring[total_len - 1] = '\0';
                }
                MPI_Gatherv(processor_name, len, MPI_CHAR, totalstring, recvcounts, displs, MPI_CHAR, 0, scomm);
                /* We get all the server names for RPC call*/
                if (server_rank == 1) {
                    /* Broadcast server_names to all processors*/
                    MPI_Bcast(&total_len, 1, MPI_INT, my_rank, MPI_COMM_WORLD);
                    final_server_list = static_cast<char *>(malloc(total_len * sizeof(char)));
                    strcpy(final_server_list, totalstring);
                    MPI_Bcast(totalstring, total_len, MPI_CHAR, my_rank, MPI_COMM_WORLD);
                    /* free data structures*/
                    free(totalstring);
                    free(displs);
                    free(recvcounts);
                } else {
                    /* Broadcast server_names to all processors*/
                    MPI_Bcast(&total_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
                    final_server_list = static_cast<char *>(malloc(total_len* sizeof(char)));
                    MPI_Bcast(final_server_list, total_len, MPI_CHAR, 0, MPI_COMM_WORLD);
                }
                server = std::make_shared<rpc::server>(server_port+my_server);
            } else {
                /* Broadcast server_names to all processors*/
                MPI_Bcast(&total_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
                final_server_list = static_cast<char *>(malloc(total_len* sizeof(char)));
                MPI_Bcast(final_server_list, total_len, MPI_CHAR, 0, MPI_COMM_WORLD);
            }
            /* Create server list from the broadcast list*/
            std::string final_server_list_str(final_server_list);
            boost::split(server_list,final_server_list_str,boost::is_any_of(","));
            isInitialized=true;
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
    template <typename F> void bind(std::string str, F func){
        server->bind(str,func);
    }

    void run(size_t workers=1){
        if(is_server) server->async_run(workers);
    }

    template <typename... Args>
    RPCLIB_MSGPACK::object_handle call(uint16_t server_index,std::string const &func_name,
                                               Args... args) {
        int16_t port = server_port + server_index;
        /* Connect to Server */
        rpc::client client(server_list[server_index].c_str(), port);
        //client.set_timeout(5000);
        return client.call(func_name, std::forward<Args>(args)...);
    }
};


#endif //HERMES_PROJECT_RPC_LIB_H

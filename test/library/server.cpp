//
// Created by manih on 4/7/2019.
//





#include <mpi.h>
#include "common.h"
#include <src/common/singleton.h>

int main(int argc, char*argv[]){
    MPI_Init(&argc,&argv);
    Input args = ParseArgs(argc,argv);
    int my_rank,comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    //FIXME: add values to CONF correctly
    auto server = Singleton<Server>::GetInstance(args.num_daemons,args.num_engines);
    server->Run();
    if (CONF->my_rank_world == 0) {
        printf("Press any key to exit server\n");
        getchar();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    server->Stop();
    MPI_Finalize();
    return 0;
}
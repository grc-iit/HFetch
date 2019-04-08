//
// Created by manih on 4/7/2019.
//

#include "common.h"

int main(int argc, char*argv[]){
    MPI_Init(&argc,&argv);
    Input args = ParseArgs(argc,argv);
    int my_rank,comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    auto server = Singleton<Server>::GetInstance();
    for(int i=0;i<args.num_events;++i){
        EventTest event;
        event.filename=CharStruct("file.bat");
        event.offset=1024*1024*i;
        event.size=1024*1024;
        event.time=i;
        server->PushEvent(event);
    }
    if (CONF->my_rank_world == 0) {
        printf("Press any key to exit server\n");
        getchar();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    server->Stop();
    MPI_Finalize();
    return 0;
}
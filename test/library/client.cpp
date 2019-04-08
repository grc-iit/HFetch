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
    //FIXME: add values to CONF correctly
    CONF;
    CONF->num_servers=args.num_servers;
    CONF->ranks_per_server=CONF->comm_size/CONF->num_servers;
    CONF->is_server=false;
    CONF->my_server=CONF->my_rank_world/CONF->ranks_per_server;
    CONF->UpdateServerComm();
    auto server = Singleton<Server>::GetInstance();
    Timer client_time;
    for(int i=0;i<args.num_events;++i){
        EventTest event;
        event.filename=CharStruct("file.bat");
        event.offset=1024*1024*i;
        event.size=1024*1024;
        event.time=i;
        client_time.resumeTime();
        server->PushEvent(event);
        client_time.pauseTime();
    }
    printf("Client %d,push time %f\n",my_rank,client_time.getTimeElapsed());
    if (CONF->my_rank_world == 0) {
        printf("Press any key to exit server\n");
        getchar();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}
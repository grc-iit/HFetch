/*
 * Copyright (C) 2019  SCS Lab <scs-help@cs.iit.edu>, Hariharan
 * Devarajan <hdevarajan@hawk.iit.edu>, Xian-He Sun <sun@iit.edu>
 *
 * This file is part of HFetch
 * 
 * HFetch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
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
    double v = client_time.getTimeElapsed();
    double sum_time;
    MPI_Allreduce(&v, &sum_time, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    if(my_rank == 0) printf("Client,push time,%f\n",sum_time/comm_size);
    if (CONF->my_rank_world == 0) {
        printf("Press any key to exit server\n");
        getchar();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}

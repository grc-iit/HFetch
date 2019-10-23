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
// Created by hariharan on 3/18/19.
//

#include <test/util.h>
#include "server.h"
#include <boost/stacktrace.hpp>

int main(int argc, char*argv[]){
    addSignals();
    std::string name="hfetch_server";
    pthread_setname_np(pthread_self(), name.c_str());
    MPI_Init(&argc,&argv);
    int my_rank,comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    /*if (my_rank == 0) {
        printf("Press any key to start server\n");
        getchar();
    }
    MPI_Barrier(MPI_COMM_WORLD);*/
    InputArgs args = Server::InitializeServer(argc,argv);
    setup_env(args);
    if(args.is_logging){
        char complete_log[256];
        sprintf(complete_log, "%s/server_%d.log", args.log_path,my_rank);
        //freopen(complete_log,"w+",stdout);
        freopen(complete_log,"w+",stderr);
    }
    Singleton<Server>::GetInstance()->async_run(args.num_workers);
    if (CONF->my_rank_world == 0) {
        printf("Press any key to exit server\n");
        getchar();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    Singleton<Server>::GetInstance()->stop();
    printf("Stopped Server:%d\n",my_rank);
    MPI_Finalize();
    return 0;
}

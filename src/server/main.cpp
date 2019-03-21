//
// Created by hariharan on 3/18/19.
//

#include "server.h"

int main(int argc, char*argv[]){

    InputArgs args = Server::InitializeServer(argc,argv);
    MPI_Init(&argc,&argv);
    Singleton<Server>::GetInstance()->async_run(args.num_workers);
    MPI_Finalize();
    return 0;
}
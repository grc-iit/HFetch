//
// Created by hariharan on 3/18/19.
//

#include <hfetch.h>
#include <mpi.h>
#include "util.h"

char* GenerateData(long size){
    char* data= static_cast<char *>(malloc(size));
    size_t num_elements=size;
    size_t offset=0;
    srand(200);
    for(int i=0;i<num_elements;++i) {
        int random = rand();
        char c = static_cast<char>((random % 26) + 'a');
        memcpy(data + offset, &c, sizeof(char));
        offset += sizeof(char);
    }
    return data;
}


int main(int argc, char*argv[]){
    InputArgs args = hfetch::MPI_Init(&argc,&argv);
    //setup_env(args);
    int my_rank,comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    if (my_rank == 0) {
        printf("Press any key to start program\n");
        getchar();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    size_t my_rank_size = args.io_size_/comm_size;
    void* buf = malloc(my_rank_size);
    char *homepath = getenv("RUN_DIR");
    char filename[256];
    char* write_buf = GenerateData(my_rank_size);
    sprintf(filename, "%s/pfs/test_%d.bat", homepath,my_rank);
    /* prepare data to be read */
    FILE* pfh = std::fopen(filename,"w+");
    std::fwrite(write_buf,my_rank_size,1,pfh);
    std::fclose(pfh);

    /* Actual APP */
    FILE* fh = hfetch::fopen(filename,"r");
    int iterations = 16;
    size_t small_io_size = my_rank_size/iterations;
    for(int i=0;i<iterations;i++){
        printf("Iteration:%d\n",i);
        hfetch::fread(buf,small_io_size,1,fh);
        usleep(10000);
    }
    hfetch::fclose(fh);
    free(buf);
    hfetch::MPI_Finalize();
    //clean_env(args);
    return 0;
}
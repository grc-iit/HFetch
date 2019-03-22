//
// Created by hariharan on 3/18/19.
//

#include <hfetch.h>

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
    void* buf = malloc(args.io_size_);
    char *homepath = getenv("RUN_DIR");
    char filename[256];
    sprintf(filename, "%s/pfs/test.bat", homepath);

    /* prepare data to be read */
    FILE* pfh = std::fopen(filename,"w+");
    char* write_buf = GenerateData(args.io_size_);
    std::fwrite(write_buf,args.io_size_,1,pfh);
    std::fclose(pfh);

    /* Actual APP */
    for(int i=0;i<1024;i++){
        printf("Iteration:%d\n",i);
        FILE* fh = hfetch::fopen(filename,"r");
        hfetch::fread(buf,args.io_size_/2,1,fh);
        hfetch::fread(buf,args.io_size_/2,1,fh);
        hfetch::fclose(fh);
        usleep(1000);
    }
    free(buf);
    hfetch::MPI_Finalize();
    return 0;
}
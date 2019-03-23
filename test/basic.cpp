//
// Created by hariharan on 3/18/19.
//

#include <hfetch.h>
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
    void* buf = malloc(args.io_size_);
    char *homepath = getenv("RUN_DIR");
    char filename[256];
    char* write_buf = GenerateData(args.io_size_);
    sprintf(filename, "%s/pfs/test_%d.bat", homepath,0);
    /* prepare data to be read */
    FILE* pfh = std::fopen(filename,"w+");
    std::fwrite(write_buf,args.io_size_,1,pfh);
    std::fclose(pfh);

    char filename2[256];
    sprintf(filename2, "%s/pfs/test_%d.bat", homepath,1);
    /* prepare data to be read */
    pfh = std::fopen(filename2,"w+");
    std::fwrite(write_buf,args.io_size_,1,pfh);
    std::fclose(pfh);

    /* Actual APP */
    FILE** fh=new FILE*[2];
    sprintf(filename, "%s/pfs/test_%d.bat", homepath,0);
    fh[0] = hfetch::fopen(filename,"r");
    sprintf(filename, "%s/pfs/test_%d.bat", homepath,1);
    fh[1] = hfetch::fopen(filename,"r");
    for(int i=0;i<8;i++){
        int index=i%2;
        printf("Iteration:%d\n",i);
        hfetch::fread(buf,args.io_size_/2,1,fh[index]);
        usleep(10000);
    }
    hfetch::fclose(fh[0]);
    hfetch::fclose(fh[1]);
    free(buf);
    hfetch::MPI_Finalize();
    //clean_env(args);
    return 0;
}
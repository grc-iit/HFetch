//
// Created by hariharan on 3/18/19.
//

#include <hfetch.h>

int main(int argc, char*argv[]){
    InputArgs args = hfetch::MPI_Init(&argc,&argv);
    void* buf = malloc(args.io_size_);
    FILE* fh = hfetch::fopen("/mnt/pfs/test.bat","r");
    hfetch::fread(buf,args.io_size_/2,1,fh);
    hfetch::fread(buf,args.io_size_/2,1,fh);
    hfetch::fclose(fh);
    fh = hfetch::fopen("/mnt/pfs/test.bat","r");
    hfetch::fread(buf,args.io_size_/2,1,fh);
    hfetch::fread(buf,args.io_size_/2,1,fh);
    hfetch::fclose(fh);
    hfetch::MPI_Finalize();
    return 0;
}
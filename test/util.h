//
// Created by hariharan on 3/18/19.
//

#ifndef HFETCH_UTIL_H
#define HFETCH_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <cstdint>
#include <src/common/data_structure.h>

int run_command(char* cmd){
    int ret;
    ret=system(cmd);
    return ret>>8;
}

static void setup_env(struct InputArgs args){
    if(args.layer_count_==0){
        char* homepath = getenv("RUN_DIR");
        ssize_t len = snprintf(NULL, 0,"mkdir -p %s/pfs %s/nvme %s/bb %s/ramfs", homepath,homepath,homepath,homepath);
        char* command=(char*)malloc(len+1);
        snprintf(command,len+1, "mkdir -p %s/pfs %s/nvme %s/bb %s/ramfs", homepath,homepath,homepath,homepath);
        run_command(command);
        ssize_t len2 = snprintf(NULL, 0, "rm -rf %s/pfs/* %s/nvme/* %s/bb/* %s/ramfs/*", homepath,homepath,homepath,homepath);
        char* command2=(char*)malloc(len2+1);
        snprintf(command2,len2+1, "rm -rf %s/pfs/* %s/nvme/* %s/bb/* %s/ramfs/*", homepath,homepath,homepath,homepath);
        run_command(command2);
    }else{
        int i;
        for(i=0;i<args.layer_count_;++i){
            char cmd1[256];
            sprintf(cmd1,"rm %s/*",args.layers[i].mount_point_);
            run_command(cmd1);
            char cmd2[256];
            sprintf(cmd2,"mkdir -p %s/",args.layers[i].mount_point_);
            run_command(cmd2);
        }
    }
}

static void clean_env(struct InputArgs args){
    if(args.layer_count_==0){
        char* homepath = getenv("RUN_DIR");
        ssize_t len2 = snprintf(NULL, 0, "rm -rf %s/pfs/* %s/nvme/* %s/bb/* %s/ramfs/*", homepath,homepath,homepath,homepath);
        char* command2=(char*)malloc(len2+1);
        snprintf(command2,len2+1, "rm -rf %s/pfs/* %s/nvme/* %s/bb/* %s/ramfs/*", homepath,homepath,homepath,homepath);
        run_command(command2);
    }else{
        int i;
        for(i=0;i<args.layer_count_;++i){
            char cmd[256];
            sprintf(cmd,"rm %s/*",args.layers[i].mount_point_);
            run_command(cmd);
        }
    }
}
#endif //HFETCH_UTIL_H

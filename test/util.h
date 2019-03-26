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

    }else{
        int i;
        for(i=0;i<args.layer_count_-1;++i){
            if(!args.layers[i].is_memory){
                char cmd1[256];
                sprintf(cmd1,"rm %s/*",args.layers[i].mount_point_);
                run_command(cmd1);
                char cmd2[256];
                sprintf(cmd2,"mkdir -p %s/",args.layers[i].mount_point_);
                run_command(cmd2);
            }
        }
    }
}

static void clean_env(struct InputArgs args){
    if(args.layer_count_==0){

    }else{
        int i;
        for(i=0;i<args.layer_count_-1;++i){
            char cmd[256];
            sprintf(cmd,"rm %s/*",args.layers[i].mount_point_);
            run_command(cmd);
        }
    }
}
#endif //HFETCH_UTIL_H

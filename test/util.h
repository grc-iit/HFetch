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

#ifndef HFETCH_UTIL_H
#define HFETCH_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <cstdint>
#include <src/common/data_structure.h>
#include <src/common/macros.h>
#include <src/common/configuration_manager.h>
#include <src/common/singleton.h>
#include <boost/filesystem.hpp>
namespace filesys = boost::filesystem;
#include <boost/filesystem.hpp>

int run_command(char* cmd){
    int ret;
    ret=system(cmd);
    return ret>>8;
}

static void setup_env(struct InputArgs args){
    if(args.layer_count_==0){

    }else{
        int i;
        for(i=0;i<args.layer_count_;++i){
            if(!args.layers[i].is_memory){
                if(args.layers[i].is_local || CONF->my_rank_server==0){
                    char cmd2[256];
                    sprintf(cmd2,"mkdir -p %s/",args.layers[i].mount_point_);
                    run_command(cmd2);
                    filesys::path pathObj(args.layers[i].mount_point_);
                    if(!filesys::is_empty(pathObj)){
                        char cmd1[256];
                        sprintf(cmd1,"rm %s/*",args.layers[i].mount_point_);
                        run_command(cmd1);
                    }



                }
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

/*
 * Copyright (C) 2019  SCS Lab <scs-help@cs.iit.edu>, Hariharan
 * Devarajan <hdevarajan@hawk.it.edu>, Xian-He Sun <sun@iit.edu>
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
// Created by hariharan on 4/1/19.
//

#include <string>
#include <cstring>
#include <mpi.h>
#include <sys/stat.h>
#include <test/util.h>

inline bool exists(const char* name) {
    struct stat buffer;
    return (stat(name, &buffer) == 0);
}
int main(int argc, char*argv[]){
    MPI_Init(&argc,&argv);
    int my_rank,comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    /*MPI_Barrier(MPI_COMM_WORLD);
    if (my_rank == 0) {
        printf("Press any key to start program\n");
        getchar();
    }
    MPI_Barrier(MPI_COMM_WORLD);*/
    int MULTIPLIER=1024;
    char *pfs_path = getenv("RUN_DIR");
    string filename = std::string(argv[0]);
    string directory;
    const size_t last_slash_idx = filename.rfind('/');
    if (std::string::npos != last_slash_idx)
    {
        directory = filename.substr(0, last_slash_idx);
    }
    std::string trace_file_name = directory+"/wrf_analysis.csv";
    std::string file_name = std::string(pfs_path)+"/pfs/wrf_analysis.dat";

    size_t readsize,len=0;
    char* line=(char*)malloc(128);
    /* Prepare data */
    FILE* trace = std::fopen(trace_file_name.c_str(), "r");
    size_t max_size=0;
    while ((readsize = getline(&line, &len, trace)) != -1) {
        if (readsize < 4) {
            break;
        }

        long offset = 0;
        long request_size = 0;
        char* word = strtok(line, ",");
        std::string operation(word);
        word = strtok(NULL, ",");
        offset = atol(word)*MULTIPLIER;
        word = strtok(NULL, ",");
        request_size = atol(word)*MULTIPLIER;
        max_size=max_size<offset+request_size?offset+request_size:max_size;
    }
    std::fclose(trace);
    max_size+=(max_size%MB);
    if(my_rank==0){
        if(!exists(file_name.c_str())){
            char command[256];
            sprintf(command,"dd if=/dev/urandom of=%s bs=%d count=%d > /dev/null 2>&1",file_name.c_str(),MB,max_size/MB);
            run_command(command);
        }
        printf("Data is prepared for the test of size %d MB\n",max_size/MB);
    }
    run_command("sudo fm");
    MPI_Barrier(MPI_COMM_WORLD);
    if (my_rank == 0) {
        printf("Press any key to start program\n");
        getchar();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    Timer t;
    t.startTime();
    t.pauseTime();
    trace = std::fopen(trace_file_name.c_str(), "r");
    MULTIPLIER=32;
    FILE* file;
    while ((readsize = getline(&line, &len, trace)) != -1) {
        if (readsize < 4) {
            break;
        }
        long offset = 0;
        long request_size = 0;
        char* word = strtok(line, ",");
        std::string operation(word);
        word = strtok(NULL, ",");
        offset = atol(word)*MULTIPLIER;
        word = strtok(NULL, ",");
        request_size = atol(word)*MULTIPLIER;
        if (operation == "FOPEN") {
            t.resumeTime();
            file = std::fopen(file_name.c_str(), "r+");
            t.pauseTime();
        } else if (operation == "FCLOSE") {
            t.resumeTime();
            std::fclose(file);
            t.pauseTime();
        } else if (operation == "FREAD") {
            char* readbuf = (char*)malloc((size_t) request_size);
            t.resumeTime();
            std::fseek(file, (size_t) offset,SEEK_SET);
            std::fread(readbuf, request_size,sizeof(char),file);
            t.pauseTime();
            if(readbuf) free(readbuf);
        } else if (operation == "LSEEK") {
            t.resumeTime();
            std::fseek(file, (size_t) offset,SEEK_SET);
            t.pauseTime();
        }
    }
    free(line);
    std::fclose(trace);
    double v = t.endTime();
    MPI_Barrier(MPI_COMM_WORLD);
    double sum_time,min_time,max_time;
    MPI_Allreduce(&v, &sum_time, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&v, &min_time, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&v, &max_time, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    double mean_time = sum_time / comm_size;
    if(my_rank == 0) {
        printf("mean_time,min_time,max_time,mean_hr,min_hr,max_hr\n");
        printf("%f,%f,%f,,,\n",mean_time/1000,min_time/1000,max_time/1000);
    }
    MPI_Finalize();
}

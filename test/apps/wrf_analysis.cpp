//
// Created by hariharan on 4/1/19.
//

#include <string>
#include <cstring>
#include <src/common/constants.h>
#include <hfetch.h>
#include <mpi.h>
#include <sys/stat.h>
#include <test/util.h>

inline bool exists(const char* name) {
    struct stat buffer;
    return (stat(name, &buffer) == 0);
}
int main(int argc, char*argv[]){
    InputArgs args = hfetch::MPI_Init(&argc,&argv);
    const int MULTIPLIER=128;
    char *pfs_path = getenv("RUN_DIR");
    std::string trace_file_name = "wrf_analysis.csv";
    std::string file_name = std::string(pfs_path)+"/pfs/wrf_analysis.dat";
    int my_rank,comm_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
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
    free(line);
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

    FILE* file;
    while ((readsize = getline(reinterpret_cast<char **>(&line), &len, trace)) != -1) {
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
            file = hfetch::fopen(file_name.c_str(), "r+");
            t.pauseTime();
        } else if (operation == "FCLOSE") {
            t.resumeTime();
            hfetch::fclose(file);
            t.pauseTime();
        } else if (operation == "FWRITE") {
            char* writebuf = (char*)calloc((size_t) request_size,sizeof(char));
            t.resumeTime();
            hfetch::fseek(file, (size_t) offset,SEEK_SET);
            hfetch::fwrite(writebuf, request_size,sizeof(char),file);
            t.pauseTime();
            if(writebuf) free(writebuf);
        }else if (operation == "FREAD") {
            char* readbuf = (char*)malloc((size_t) request_size);
            t.resumeTime();
            hfetch::fseek(file, (size_t) offset,SEEK_SET);
            hfetch::fread(readbuf, request_size,sizeof(char),file);
            t.pauseTime();
            if(readbuf) free(readbuf);
        } else if (operation == "LSEEK") {
            t.resumeTime();
            hfetch::fseek(file, (size_t) offset,SEEK_SET);
            t.pauseTime();
        }
    }
    std::fclose(trace);
    double v = t.endTime();
    MPI_Barrier(MPI_COMM_WORLD);
    printf("rank:%d hit ratio %f time %f\n",my_rank,CONF->hit/(CONF->total*1.0),v);
    hfetch::MPI_Finalize();
}
//
// Created by hariharan on 3/18/19.
//

#ifndef HFETCH_INT_UTIL_H
#define HFETCH_INT_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <cstdint>
#include <src/common/data_structure.h>
static char** str_split(char* a_str, const char a_delim){
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char**)malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
static struct InputArgs parse_opts(int argc, char *argv[]){
    int flags, opt;
    int nsecs, tfnd;

    nsecs = 0;
    tfnd = 0;
    flags = 0;
    struct InputArgs args;
    args.io_size_=0;
    args.layer_count_=0;
    args.pfs_path=NULL;
    args.iteration_=1;
    args.direct_io_=true;
    args.ranks_per_server_=1;
    while ((opt = getopt (argc, argv, "l:i:f:n:d:r:w:")) != -1)
    {
        switch (opt)
        {
            case 'l':{
                char** layers=str_split(optarg,'#');
                int layer_count=atoi(layers[0]);
                LayerInfo* layerInfos=(LayerInfo*)malloc(sizeof(LayerInfo)*layer_count);
                int i;
                for(i=0;i<layer_count;++i){
                    char** layerInfoStr=str_split(layers[i+1],'_');
                    layerInfos[i].capacity_mb_= (float) atof(layerInfoStr[0]);
                    layerInfos[i].bandwidth= (float) atof(layerInfoStr[1]);
                    layerInfos[i].is_memory= (bool) atoi(layerInfoStr[2]);
                    strcpy(layerInfos[i].mount_point_, layerInfoStr[3]);
                    layerInfos[i].direct_io= (bool) atoi(layerInfoStr[4]);
                }
                args.layer_count_=layer_count;
                args.layers=layerInfos;
                break;
            }
            case 'i':{
                args.io_size_= (size_t) atoi(optarg);
                break;
            }
            case 'r':{
                args.ranks_per_server_= (size_t) atoi(optarg);
                break;
            }
            case 'n':{
                args.iteration_= (size_t) atoi(optarg);
                break;
            }
            case 'f':{
                args.pfs_path= optarg;
                break;
            }
            case 'd':{
                args.direct_io_= static_cast<size_t>(atoi(optarg));
                break;
            }
            case 'w':{
                args.num_workers= static_cast<size_t>(atoi(optarg));
                break;
            }
            default:               /* '?' */
                fprintf (stderr, "Usage: %s [-l layer_count;l(i)_capacity_mb-l(i)_bandwidth-l(i)_is_memory-l(i)_mount_point] [-i io_size_per_request]  [-f pfs_path] [-d direct io true/false] [-n request repetition] [-r ranks_per_server] [-w num_workers]\n", argv[0]);
                exit (EXIT_FAILURE);
        }
    }
    return args;
}
#endif //HFETCH_UTIL_H

//
// Created by hariharan on 3/18/19.
//

#ifndef HFETCH_HFETCH_H
#define HFETCH_HFETCH_H

#include <cstdio>
#include <src/common/data_structure.h>

namespace hfetch {
/******************************************************************************
*Interface operations
******************************************************************************/
    InputArgs MPI_Init( int *argc, char ***argv );

    FILE *fopen(const char *filename, const char *mode);

    int fclose(FILE *stream);

    int fseek(FILE *stream, long int offset, int origin);

    size_t fread(void *ptr, size_t size, size_t count, FILE *stream);

    size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);

    int MPI_Finalize();

}
#endif //HFETCH_HFETCH_H

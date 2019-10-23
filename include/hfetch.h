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

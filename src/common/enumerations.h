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
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_ENUMERATIONS_H
#define HFETCH_ENUMERATIONS_H
#include <rpc/msgpack.hpp>

/**
 * Defines the enum used in HFetch
 */

/* Enumerates Server Status */
typedef enum ServerStatus{
    SERVER_SUCCESS=0,
    SERVER_FAILED=1,
} ServerStatus;

/* Enumerates Event Type */
typedef enum EventType{
    FILE_OPEN=0,
    FILE_CLOSE=1,
    FILE_READ=2,
    FILE_WRITE=3
} EventType;

typedef enum ScoreType{
    FREQUENCY_SCORE=0,
    LRF_SCORE=1
};

/* Enumerates Event Source */
typedef enum EventSource{
    APPLICATION=0,
    HARDWARE=1
} EventSource;

/* Enumerates Data Placement Engines */
typedef enum DataPlacementEngineType{
    MAX_BW=0
} DataPlacementEngineType;


/* IOClient defines various types of IO Clients supported by HFetch */
typedef enum IOClientType{
    LOCAL_POSIX_FILE=0,
    SIMPLE_MEMORY=1,
    SHARED_POSIX_FILE=2
} IOClientType;

/**
 * MSGPACK
 */

MSGPACK_ADD_ENUM(ServerStatus);
MSGPACK_ADD_ENUM(EventType);
MSGPACK_ADD_ENUM(EventSource);
MSGPACK_ADD_ENUM(IOClientType);
MSGPACK_ADD_ENUM(DataPlacementEngineType);



#endif //HFETCH_ENUMERATIONS_H

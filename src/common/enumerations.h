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
    POSIX_FILE=0,
    SIMPLE_MEMORY=1
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

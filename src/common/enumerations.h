//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_ENUMERATIONS_H
#define HFETCH_ENUMERATIONS_H

typedef enum ServerStatus{
    SERVER_SUCCESS=0,
    SERVER_FAILED=1,
} ServerStatus;

typedef enum EventType{
    FILE_OPEN=0,
    FILE_CLOSE=1,
    FILE_READ=2,
    FILE_WRITE=3
} EventType;

/**
 * IOClient defines various types of IO Clients supported by Hermes
 */
typedef enum IOClientType{
    POSIX_FILE=0,
    SIMPLE_MEMORY=1
} IOClientType;


#endif //HFETCH_ENUMERATIONS_H

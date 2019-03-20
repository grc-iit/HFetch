//
// Created by hariharan on 3/19/19.
//

#include "memory_client.h"

ServerStatus MemoryClient::Read(PosixFile source, PosixFile destination) {
    return SERVER_FAILED;
}

ServerStatus MemoryClient::Write(PosixFile source, PosixFile destination) {
    return SERVER_FAILED;
}

ServerStatus MemoryClient::Delete(PosixFile file) {
    return SERVER_FAILED;
}

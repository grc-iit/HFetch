//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_MEMORY_CLIENT_H
#define HFETCH_MEMORY_CLIENT_H


#include "io_client.h"

class MemoryClient: public IOClient {
public:
    ServerStatus Read(PosixFile source, PosixFile destination) override;
    ServerStatus Write(PosixFile source, PosixFile destination) override;

    ServerStatus Delete(PosixFile file) override;
};


#endif //HFETCH_MEMORY_CLIENT_H

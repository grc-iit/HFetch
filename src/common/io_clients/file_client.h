//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_FILE_CLIENT_H
#define HFETCH_FILE_CLIENT_H


#include "io_client.h"

class FileClient: public IOClient {
public:
    ServerStatus Read(PosixFile source, PosixFile destination) override;
    ServerStatus Write(PosixFile source, PosixFile destination) override;
    ServerStatus Delete(PosixFile file) override;

    double GetCurrentUsage(Layer l) override;
};


#endif //HFETCH_FILE_CLIENT_H

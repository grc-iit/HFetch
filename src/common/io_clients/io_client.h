//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_IO_CLIENT_H
#define HFETCH_IO_CLIENT_H

#include <src/common/enumerations.h>
#include <src/common/data_structure.h>

class IOClient{
public:
    IOClient(){}
    virtual ServerStatus Read(PosixFile source, PosixFile destination) = 0;
    virtual ServerStatus Write(PosixFile source, PosixFile destination) = 0;
    virtual ServerStatus Delete(PosixFile file) = 0;

};
#endif //HFETCH_IO_CLIENT_H

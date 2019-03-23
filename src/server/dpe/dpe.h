//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_DPE_H
#define HFETCH_DPE_H

#include <src/common/data_structure.h>

class DPE{
public:
    virtual std::vector<std::tuple<PosixFile, PosixFile,double>> place(std::vector<Event> events) = 0;
};
#endif //HFETCH_DPE_H

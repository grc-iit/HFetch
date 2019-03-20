//
// Created by hariharan on 3/20/19.
//

#ifndef HFETCH_MAX_BW_DPE_H
#define HFETCH_MAX_BW_DPE_H


#include "dpe.h"

class MaxBandwidthDPE: public DPE {
public:
    std::vector<std::pair<PosixFile,PosixFile>> place(std::vector<Event> events) override;
};


#endif //HFETCH_MAX_BW_DPE_H

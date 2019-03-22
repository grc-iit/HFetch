//
// Created by hariharan on 3/20/19.
//

#ifndef HFETCH_MAX_BW_DPE_H
#define HFETCH_MAX_BW_DPE_H


#include <src/server/file_segment_auditor.h>
#include "dpe.h"

class MaxBandwidthDPE: public DPE {
    std::shared_ptr<FileSegmentAuditor> fileSegmentAuditor;
public:
    MaxBandwidthDPE(){
        fileSegmentAuditor = Singleton<FileSegmentAuditor>::GetInstance();
    }
    std::vector<std::pair<PosixFile,PosixFile>> place(std::vector<Event> events) override;

    std::vector<std::pair<PosixFile, PosixFile>> solve(std::tuple<Segment,SegmentScore, PosixFile> segment_tuple,
                                                       const std::map<uint8_t, std::tuple<double, double,double>>* layerScore,
                                                       Layer* layer,
                                                       size_t* used_capacities,
                                                       long original_index=0);

    CharStruct GenerateBufferFilename();

    bool IsBetween(double score, double min_score, double max_score);

    std::vector<PosixFile> Split(PosixFile segment, long d);
};


#endif //HFETCH_MAX_BW_DPE_H

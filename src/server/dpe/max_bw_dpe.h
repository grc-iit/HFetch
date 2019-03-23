//
// Created by hariharan on 3/20/19.
//

#ifndef HFETCH_MAX_BW_DPE_H
#define HFETCH_MAX_BW_DPE_H


#include <src/server/file_segment_auditor.h>
#include "dpe.h"

class MaxBandwidthDPE: public DPE {
    std::shared_ptr<FileSegmentAuditor> fileSegmentAuditor;
    std::shared_ptr<IOClientFactory> ioFactory;
public:
    MaxBandwidthDPE(){
        fileSegmentAuditor = Singleton<FileSegmentAuditor>::GetInstance();
        ioFactory = Singleton<IOClientFactory>::GetInstance();
    }
    std::vector<std::tuple<PosixFile, PosixFile,double>> place(std::vector<Event> events) override;

    std::vector<std::tuple<PosixFile, PosixFile,double>> solve(std::tuple<Segment,SegmentScore, PosixFile> segment_tuple,
                                                       const std::map<uint8_t, std::multimap<double,std::pair<PosixFile,PosixFile>,std::greater<double>>>* layerScore,
                                                       Layer* layer,
                                                       long original_index=0);

    CharStruct GenerateBufferFilename();

    bool IsAllowed(double score, double min_score, double max_score);

    std::vector<PosixFile> Split(PosixFile segment, long d);
};


#endif //HFETCH_MAX_BW_DPE_H

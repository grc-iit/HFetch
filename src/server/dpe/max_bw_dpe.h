//
// Created by hariharan on 3/20/19.
//

#ifndef HFETCH_MAX_BW_DPE_H
#define HFETCH_MAX_BW_DPE_H


#include <src/server/file_segment_auditor.h>
#include <src/common/io_clients/data_manager.h>
#include "dpe.h"

class MaxBandwidthDPE: public DPE {
    std::shared_ptr<FileSegmentAuditor> fileSegmentAuditor;
    std::shared_ptr<IOClientFactory> ioFactory;
    std::shared_ptr<DataManager> dataManager;
public:
    MaxBandwidthDPE(){
        fileSegmentAuditor = Singleton<FileSegmentAuditor>::GetInstance();
        ioFactory = Singleton<IOClientFactory>::GetInstance();
        dataManager = Singleton<DataManager>::GetInstance();
    }
    std::vector<std::tuple<PosixFile, PosixFile,double>> place(std::vector<Event> events) override;

    std::vector<std::tuple<PosixFile, PosixFile,double>> solve(std::tuple<Segment,SegmentScore, PosixFile> segment_tuple,
                                                       const std::map<uint8_t, std::multimap<double,std::pair<PosixFile,PosixFile>,std::greater<double>>>* layerScore,
                                                       Layer* layer,
                                                       long original_index=0);

    bool IsAllowed(double score, double min_score, double max_score);
};


#endif //HFETCH_MAX_BW_DPE_H

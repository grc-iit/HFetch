/*
 * Copyright (C) 2019  SCS Lab <scs-help@cs.iit.edu>, Hariharan
 * Devarajan <hdevarajan@hawk.it.edu>, Xian-He Sun <sun@iit.edu>
 *
 * This file is part of HFetch
 * 
 * HFetch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
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

    std::vector<pair<PosixFile, PosixFile>> PlaceDataInLayer(PosixFile &file, Layer &layer, long &original_index);

    vector<PosixFile> SplitInParts(PosixFile file, bool b);
};


#endif //HFETCH_MAX_BW_DPE_H

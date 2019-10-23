/*
 * Copyright (C) 2019  SCS Lab <scs-help@cs.iit.edu>, Hariharan
 * Devarajan <hdevarajan@hawk.iit.edu>, Xian-He Sun <sun@iit.edu>
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

#include "max_bw_dpe.h"
#include <tuple>

std::vector<std::tuple<PosixFile, PosixFile,double>> MaxBandwidthDPE::place(std::vector<Event> events) {
    AutoTrace trace = AutoTrace("MaxBandwidthDPE::place",events);
    auto layerScore = fileSegmentAuditor->FetchLayerScores();
    auto total_placements = std::vector<std::tuple<PosixFile, PosixFile,double>>();
    for(auto event:events){
        if(event.event_type!=EventType::FILE_CLOSE){
            PosixFile file;
            file.filename=event.filename;
            file.segment=event.segment;
            file.layer=Layer(event.layer_index);
            auto heatMap = fileSegmentAuditor->FetchHeatMap(file);
            for(auto segment_tuple:heatMap){
                auto placements = solve(segment_tuple,&layerScore,Layer::FIRST);
                total_placements.insert(total_placements.end(),placements.begin(),placements.end());
            }
        }
    }
    return total_placements;
}

std::vector<std::tuple<PosixFile, PosixFile,double>>
MaxBandwidthDPE::solve(std::tuple<Segment,SegmentScore, PosixFile> segment_tuple,
                        const std::map<uint8_t, std::multimap<double,std::pair<PosixFile,PosixFile>,std::greater<double>>>* layerScore,
                        Layer* layer,
                        long original_index) {
    AutoTrace trace = AutoTrace("MaxBandwidthDPE::solve",std::get<0>(segment_tuple),std::get<1>(segment_tuple),std::get<2>(segment_tuple),*layer);
    double remaining_capacity=layer->capacity_mb_*MB-ioFactory->GetClient(layer->io_client_type)->GetCurrentUsage(*layer);
    remaining_capacity=remaining_capacity<0?0:remaining_capacity;
    auto final_vector = std::vector<std::tuple<PosixFile, PosixFile,double>>();
    Segment segment = std::get<0>(segment_tuple);
    SegmentScore score_obj = std::get<1>(segment_tuple);
    double score = score_obj.GetScore();
    PosixFile current_file = std::get<2>(segment_tuple);
    auto layerScoreMap = layerScore->find(layer->id_)->second;
    double layer_min_score = -1*(std::numeric_limits<double>::max()-1);
    double layer_max_score = std::numeric_limits<double>::max();
    if(layerScoreMap.size() > 0){
        layer_min_score = layerScoreMap.begin()->first;
        layer_max_score = layerScoreMap.begin()->first;
    }
    if(layerScoreMap.size() > 1) layer_min_score = layerScoreMap.rbegin()->first;
    /* if data already in this layer dont bother. */
    if(current_file.layer == *layer) return final_vector;
    /* if its last layer data is already there as it is prefetching */
    else if(layer->id_==Layer::LAST->id_) return final_vector;
    else if(remaining_capacity >= segment.GetSize()){
        /* Fits and score is fine too*/
        if(current_file.layer != *layer){
            auto placements=this->PlaceDataInLayer(current_file,*layer,original_index);
            for(auto placement:placements){
                final_vector.push_back(std::tuple<PosixFile, PosixFile,double>(placement.first,placement.second,score));
            }

        }
    }else if(IsAllowed(score, layer_min_score, layer_max_score)){
        /* if the score is > min in layer
         * Find how much data needs to be shifted to accommodate this layer.
         * we start from end keep adding sizes
         * The following cases would occur
         * 1. if enough data has scores less than current data has then space_avail >=current_file.GetSize()
         * will hit which means we can fit all data with eviction to some data
         * 2. if we dont have enough data less than current data score then iter->first < score will hit
         * which means that some portion of data will fit in current layer and rest has to go to next
         * */
        long space_avail = remaining_capacity;
        auto iter = layerScoreMap.end();
        while(iter!=layerScoreMap.begin() && iter->first < score && space_avail <current_file.GetSize()){
            space_avail += iter->second.second.GetSize();
            iter--;
        }
        if(space_avail > 0){
            /* means case 1 and 2 from above*/
            if(space_avail >= current_file.GetSize()){
                /* case 1*/
                auto placements=this->PlaceDataInLayer(current_file,*layer,original_index);
                for(auto placement:placements)
                    final_vector.push_back(std::tuple<PosixFile, PosixFile,double>(placement.first,placement.second,score));
            }else{
                /* case 2 */
                std::vector<PosixFile> pieces = dataManager->Split(current_file,space_avail);
                if(current_file.layer != *layer){
                    auto placements=this->PlaceDataInLayer(pieces[0],*layer,original_index);
                    for(auto placement:placements)
                        final_vector.push_back(std::tuple<PosixFile, PosixFile,double>(placement.first,placement.second,score));
                }
                auto sub_problem = solve(std::tuple<Segment,SegmentScore,PosixFile>(pieces[1].segment,score_obj,pieces[1]),layerScore,layer->next,original_index);
                final_vector.insert(final_vector.end(),sub_problem.begin(),sub_problem.end());
            }
        }
    }else{
        /* do next layer */
        auto sub_problem=solve(segment_tuple,layerScore,layer->next,original_index);
        final_vector.insert(final_vector.end(),sub_problem.begin(),sub_problem.end());
    }

    return final_vector;
}

bool MaxBandwidthDPE::IsAllowed(double score, double min_score, double max_score) {
    AutoTrace trace = AutoTrace("MaxBandwidthDPE::IsAllowed",score,min_score,max_score);
    return score > min_score;
}

std::vector<pair<PosixFile, PosixFile>> MaxBandwidthDPE::PlaceDataInLayer(PosixFile &current_file, Layer &layer, long &original_index) {
    PosixFile source = current_file;
    PosixFile destination = source;
    destination.layer=layer;
    destination.segment.start = 0;
    destination.segment.end = source.GetSize() - 1;
    source.segment.start = original_index;
    source.segment.end = original_index + source.GetSize() - 1;
    original_index = source.segment.end + 1;
    std::vector<PosixFile> original_pieces=this->SplitInParts(source,false);
    std::vector<PosixFile> buf_pieces=SplitInParts(destination,current_file.layer == *Layer::LAST);
    auto placements = std::vector<pair<PosixFile, PosixFile>>();
    for(int i=0;i<buf_pieces.size();++i){
        placements.push_back(pair<PosixFile, PosixFile>(original_pieces[i],buf_pieces[i]));
    }
    return placements;
}

vector<PosixFile> MaxBandwidthDPE::SplitInParts(PosixFile file, bool generateName) {
    int parts=file.GetSize()/SEGMENT_SIZE;
    parts += (file.GetSize()%SEGMENT_SIZE==0?0:1);
    int original_index=file.segment.start;
    int left_size=file.GetSize();
    vector<PosixFile> pieces = vector<PosixFile>();

    for(int i=0;i<parts;++i){
        PosixFile piece=file;
        piece.segment.start=original_index;
        piece.segment.end=original_index + (left_size<SEGMENT_SIZE?left_size:SEGMENT_SIZE)-1;
        if(generateName) piece.filename = dataManager->GenerateBufferFilename();
        if(!generateName)  original_index = piece.segment.end+1;
        pieces.push_back(piece);
        left_size-=piece.GetSize();
    }
    return pieces;
}


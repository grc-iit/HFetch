//
// Created by hariharan on 3/20/19.
//

#include "max_bw_dpe.h"
#include <tuple>

std::vector<std::tuple<PosixFile, PosixFile,double>> MaxBandwidthDPE::place(std::vector<Event> events) {
    auto layerScore = fileSegmentAuditor->FetchLayerScores();
    auto total_placements = std::vector<std::tuple<PosixFile, PosixFile,double>>();
    for(auto event:events){
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
    return total_placements;
}

CharStruct MaxBandwidthDPE::GenerateBufferFilename() {
    /* use timestamp to generate unique file names. */
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int us = tp.tv_sec * 1000000 + tp.tv_usec;
    return CharStruct(std::to_string(us) + ".h5");
}

std::vector<std::tuple<PosixFile, PosixFile,double>>
MaxBandwidthDPE::solve(std::tuple<Segment,SegmentScore, PosixFile> segment_tuple,
                        const std::map<uint8_t, std::multimap<double,std::pair<PosixFile,PosixFile>,std::greater<double>>>* layerScore,
                        Layer* layer,
                        long original_index) {
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
            PosixFile source = current_file;
            PosixFile destination = source;
            destination.layer=*layer;
            if(current_file.layer == *Layer::LAST) destination.filename = GenerateBufferFilename();
            destination.segment.start = 0;
            destination.segment.end = source.GetSize() - 1;
            source.segment.start = original_index;
            source.segment.end = original_index + source.GetSize() - 1;
            original_index = original_index + source.GetSize()  - 1;
            final_vector.push_back(std::tuple<PosixFile, PosixFile,double>(source,destination,score));
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
                PosixFile source = current_file;
                PosixFile destination = source;
                destination.layer=*layer;
                if(current_file.layer == *Layer::LAST) destination.filename = GenerateBufferFilename();
                destination.segment.start = 0;
                destination.segment.end = source.GetSize() - 1;
                source.segment.start = original_index;
                source.segment.end = original_index + source.GetSize() - 1;
                original_index = original_index + source.GetSize() - 1;
                final_vector.push_back(std::tuple<PosixFile, PosixFile,double>(source,destination,score));
            }else{
                /* case 2 */
                std::vector<PosixFile> pieces = Split(current_file,space_avail);
                if(current_file.layer != *layer){
                    PosixFile source = pieces[0];
                    PosixFile destination = source;
                    destination.layer=*layer;
                    destination.segment.start = 0;
                    destination.segment.end = source.GetSize() - 1;
                    source.segment.start = original_index;
                    source.segment.end = original_index + source.GetSize() - 1;
                    original_index = original_index + source.GetSize() - 1;
                    if(current_file.layer == *Layer::LAST) destination.filename = GenerateBufferFilename();
                    final_vector.push_back(std::tuple<PosixFile, PosixFile,double>(source,destination,score));
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
    return score > min_score;
}

std::vector<PosixFile> MaxBandwidthDPE::Split(PosixFile file, long remaining_capacity) {
    std::vector<PosixFile> pieces=std::vector<PosixFile>();
    PosixFile p1=file;
    p1.segment.end = p1.segment.start + remaining_capacity - 1;
    PosixFile p2=file;
    p2.segment.start = p1.segment.start + remaining_capacity;
    pieces.push_back(p1);
    pieces.push_back(p2);
    return pieces;
}

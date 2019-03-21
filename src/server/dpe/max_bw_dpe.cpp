//
// Created by hariharan on 3/20/19.
//

#include "max_bw_dpe.h"
#include <tuple>

std::vector<std::pair<PosixFile,PosixFile>> MaxBandwidthDPE::place(std::vector<Event> events) {
    auto layerScore = fileSegmentAuditor->FetchLayerScores();
    std::vector<std::pair<PosixFile,PosixFile>> total_placements = std::vector<std::pair<PosixFile,PosixFile>>();
    for(auto event:events){
        PosixFile file;
        file.filename=event.filename;
        file.segment=event.segment;
        file.layer=Layer(event.layer_index);
        auto heatMap = fileSegmentAuditor->FetchHeatMap(file);
        std::vector<std::pair<PosixFile,PosixFile>> placements = solve(&heatMap,&layerScore,Layer::FIRST);
        total_placements.insert(total_placements.end(),placements.begin(),placements.end());
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

std::vector<std::pair<PosixFile, PosixFile>>
MaxBandwidthDPE::solve( const std::vector<std::tuple<Segment,SegmentScore, PosixFile>>* heatMap,
                        const std::map<uint8_t, std::tuple<double, double,double>>* layerScore,
                        Layer* layer, size_t placed_index,
                        size_t used_capacity) {
    std::vector<std::pair<PosixFile, PosixFile>> final_vector = std::vector<std::pair<PosixFile, PosixFile>>();
    for(size_t i=placed_index;i<heatMap->size();++i){
        Segment segment = std::get<0>(heatMap->at(i));
        SegmentScore score_obj = std::get<1>(heatMap->at(i));
        double score = score_obj.GetScore();
        PosixFile current_file = std::get<2>(heatMap->at(i));
        double layer_min_score = std::get<0>(layerScore->find(layer->id_)->second);
        double layer_max_score = std::get<1>(layerScore->find(layer->id_)->second);
        double remaining_capacity = std::get<2>(layerScore->find(layer->id_)->second);
        if(remaining_capacity-used_capacity > segment.GetSize() ||layer->id_==Layer::LAST->id_){
            /* Fits and score is fine too*/
            if(current_file.layer != *layer){
                PosixFile source = current_file;
                PosixFile destination = source;
                if(current_file.layer == *Layer::LAST) destination.filename = GenerateBufferFilename();
                final_vector.push_back(std::pair<PosixFile, PosixFile>(source,destination));
                used_capacity += segment.GetSize();
            }
        }else{
            if(IsBetween(score,layer_min_score,layer_max_score)){
                if(current_file.layer != *layer){
                    PosixFile source = current_file;
                    PosixFile destination = source;
                    if(current_file.layer == *Layer::LAST) destination.filename = GenerateBufferFilename();
                    final_vector.push_back(std::pair<PosixFile, PosixFile>(source,destination));
                    used_capacity += segment.GetSize();
                }
            }else{
                /* do next layer */
                auto sub_problem=solve(heatMap,layerScore,layer->next,i,used_capacity);
                final_vector.insert(final_vector.end(),sub_problem.begin(),sub_problem.end());
                break;
            }
        }
    }
    return std::vector<std::pair<PosixFile, PosixFile>>();
}

bool MaxBandwidthDPE::IsBetween(double score, double min_score, double max_score) {
    return score >= min_score && score<=max_score;
}

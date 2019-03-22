//
// Created by hariharan on 3/20/19.
//

#include "max_bw_dpe.h"
#include <tuple>

std::vector<std::pair<PosixFile,PosixFile>> MaxBandwidthDPE::place(std::vector<Event> events) {
    auto layerScore = fileSegmentAuditor->FetchLayerScores();
    std::vector<std::pair<PosixFile,PosixFile>> total_placements = std::vector<std::pair<PosixFile,PosixFile>>();
    size_t* used_capacities=new size_t[layerScore.size()];
    for(int i=0;i<layerScore.size();++i){
        used_capacities[i]=0;
    }
    for(auto event:events){
        PosixFile file;
        file.filename=event.filename;
        file.segment=event.segment;
        file.layer=Layer(event.layer_index);
        auto heatMap = fileSegmentAuditor->FetchHeatMap(file);
        for(auto segment:heatMap){
            std::vector<std::pair<PosixFile,PosixFile>> placements = solve(segment,&layerScore,Layer::FIRST,used_capacities);
            total_placements.insert(total_placements.end(),placements.begin(),placements.end());
        }

    }
    delete(used_capacities);
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
MaxBandwidthDPE::solve( std::tuple<Segment,SegmentScore, PosixFile> segment_tuple,
                        const std::map<uint8_t, std::tuple<double, double,double>>* layerScore,
                        Layer* layer,
                        size_t* used_capacities,
                        long original_index) {
    std::vector<std::pair<PosixFile, PosixFile>> final_vector = std::vector<std::pair<PosixFile, PosixFile>>();
    Segment segment = std::get<0>(segment_tuple);
    SegmentScore score_obj = std::get<1>(segment_tuple);
    double score = score_obj.GetScore();
    PosixFile current_file = std::get<2>(segment_tuple);
    double layer_min_score = std::get<0>(layerScore->find(layer->id_)->second);
    double layer_max_score = std::get<1>(layerScore->find(layer->id_)->second);
    long remaining_capacity = std::get<2>(layerScore->find(layer->id_)->second);
    if(remaining_capacity-used_capacities[layer->id_-1] > segment.GetSize() ||layer->id_==Layer::LAST->id_){
        /* Fits and score is fine too*/
        if(current_file.layer != *layer){
            PosixFile source = current_file;
            PosixFile destination = source;
            destination.layer=*layer;
            if(current_file.layer == *Layer::LAST) destination.filename = GenerateBufferFilename();
            destination.segment.start = 0;
            destination.segment.end = source.GetSize();
            source.segment.start = original_index;
            source.segment.end = original_index + source.GetSize();
            original_index = original_index + source.GetSize();
            final_vector.push_back(std::pair<PosixFile, PosixFile>(source,destination));
            used_capacities[layer->id_-1] += segment.GetSize();
        }
    }else if(IsBetween(score,layer_min_score,layer_max_score)){
        std::vector<PosixFile> pieces = Split(current_file,remaining_capacity-used_capacities[layer->id_-1]);
        if(current_file.layer != *layer){
            PosixFile source = pieces[0];
            PosixFile destination = source;
            destination.segment.start = 0;
            destination.segment.end = source.GetSize();
            source.segment.start = original_index;
            source.segment.end = original_index + source.GetSize();
            original_index = original_index + source.GetSize();
            if(current_file.layer == *Layer::LAST) destination.filename = GenerateBufferFilename();
            final_vector.push_back(std::pair<PosixFile, PosixFile>(source,destination));
            used_capacities[layer->id_-1] += source.GetSize();
        }
        auto sub_problem = solve(std::tuple<Segment,SegmentScore,PosixFile>(pieces[1].segment,score_obj,pieces[1]),layerScore,layer->next,used_capacities,original_index);
        final_vector.insert(final_vector.end(),sub_problem.begin(),sub_problem.end());
    }else{
        /* do next layer */
        auto sub_problem=solve(segment_tuple,layerScore,layer->next,used_capacities,original_index);
        final_vector.insert(final_vector.end(),sub_problem.begin(),sub_problem.end());
    }

    return final_vector;
}

bool MaxBandwidthDPE::IsBetween(double score, double min_score, double max_score) {
    return score >= min_score && score<=max_score;
}

std::vector<PosixFile> MaxBandwidthDPE::Split(PosixFile file, long remaining_capacity) {
    std::vector<PosixFile> pieces=std::vector<PosixFile>();
    PosixFile p1=file;
    p1.segment.end = p1.segment.start + remaining_capacity;
    PosixFile p2=file;
    p2.segment.start = p1.segment.start + remaining_capacity + 1;
    pieces.push_back(p1);
    pieces.push_back(p2);
    return pieces;
}

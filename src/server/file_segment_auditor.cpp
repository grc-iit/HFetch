//
// Created by hariharan on 3/19/19.
//

#include "file_segment_auditor.h"


ServerStatus FileSegmentAuditor::Update(std::vector<Event> events) {
    for(auto event : events){
        switch(event.event_type){
            case EventType::FILE_OPEN:{
                auto file_iter = file_segment_map.find(event.filename);
                if(file_iter == file_segment_map.end()){
                    SyncCreateOffsetMap(event);
                }
                break;
            }
        }
        switch(event.source){
            case EventSource::APPLICATION:{
                switch(event.event_type){
                    case EventType::FILE_OPEN:{
                        MarkFileSegmentsActive(event);
                        break;
                    }
                    case EventType::FILE_CLOSE:{
                        MarkFileSegmentsInactive(event);
                        break;
                    }
                }
                break;
            }
            case EventSource::HARDWARE:{
                switch(event.event_type){
                    case EventType::FILE_OPEN:{
                        MarkFileSegmentsActive(event);
                        break;
                    }
                    case EventType::FILE_CLOSE:{
                        MarkFileSegmentsInactive(event);
                        break;
                    }
                    case EventType::FILE_READ:{
                        IncreaseFileSegmentFrequency(event);
                        break;
                    }
                }
                break;
            }
        }
    }
    return SERVER_SUCCESS;
}

ServerStatus FileSegmentAuditor::UpdateOnPrefetch(PosixFile source,PosixFile destination) {
    auto iter = file_segment_map.find(source.filename);
    if(iter != file_segment_map.end()){
        SegmentMap* multiMapScore = iter->second;
        auto iter = multiMapScore->Get(source.segment);
        if(iter.first){
            iter.second.first=destination;
            multiMapScore->Put(source.segment,iter.second);
        }else{
            auto allDatas = multiMapScore->Contains(source.segment);
            for(auto elements : allDatas){
                multiMapScore->Erase(elements.first);
                /* retain left over scores */
                auto left_overs = source.segment.Substract(elements.first);
                for(auto left_over : left_overs){
                    multiMapScore->Put(left_over,elements.second);
                }
                /* update intersected score */
                auto common = source.segment.Intersect(elements.first);
                elements.second.first = destination;
                elements.second.first.segment.start = common.start - source.segment.start;
                elements.second.first.segment.end = common.end - source.segment.start;
                multiMapScore->Put(common,elements.second);
            }
        }
    }
    return SERVER_SUCCESS;
}

ServerStatus FileSegmentAuditor::MarkFileSegmentsActive(Event event) {
    auto iter = file_active_status.Get(event.filename);
    if(iter.first){
        file_active_status.Put(event.filename,iter.second + 1);
    }else{
        file_active_status.Put(event.filename,1);
    }
    return SERVER_SUCCESS;
}

ServerStatus FileSegmentAuditor::MarkFileSegmentsInactive(Event event) {
    auto iter = file_active_status.Get(event.filename);
    if(iter.first){
        file_active_status.Put(event.filename,iter.second - 1);
    }
    return SERVER_SUCCESS;
}

ServerStatus FileSegmentAuditor::IncreaseFileSegmentFrequency(Event event) {
    auto iter = file_segment_map.find(event.filename);
    if(iter != file_segment_map.end()){
        SegmentMap* multiMapScore = iter->second;
        auto iter = multiMapScore->Get(event.segment);
        if(iter.first){
            iter.second.second.frequency+=1;
            iter.second.second.time+=event.time;
            double newScore = iter.second.second.GetScore();
            auto layer_score_iter = layer_score_map.Get(iter.second.first.layer.id_);
            if(layer_score_iter.first){
                double max_score=layer_score_iter.second.second,min_score=layer_score_iter.second.first;
                if(layer_score_iter.second.second < newScore) max_score=newScore;
                if(layer_score_iter.second.first > newScore) min_score=newScore;
                layer_score_map.Put(iter.second.first.layer.id_,std::pair<double,double>(min_score,max_score));
            }else{
                layer_score_map.Put(iter.second.first.layer.id_,std::pair<double,double>(newScore,newScore));
            }
            multiMapScore->Put(event.segment,iter.second);
        }else{
            auto allDatas = multiMapScore->Contains(event.segment);
            for(auto elements : allDatas){
                multiMapScore->Erase(elements.first);
                /* retain left over scores */
                auto left_overs = event.segment.Substract(elements.first);
                for(auto left_over : left_overs){
                    multiMapScore->Put(left_over,elements.second);
                }
                /* update intersected score */
                auto common = event.segment.Intersect(elements.first);
                elements.second.second.frequency+=1;
                elements.second.second.time+=event.time;
                double newScore = elements.second.second.GetScore();
                auto layer_score_iter = layer_score_map.Get(elements.second.first.layer.id_);
                if(layer_score_iter.first){
                    double max_score=layer_score_iter.second.second,min_score=layer_score_iter.second.first;
                    if(layer_score_iter.second.second < newScore) max_score=newScore;
                    if(layer_score_iter.second.first > newScore) min_score=newScore;
                    layer_score_map.Put(elements.second.first.layer.id_,std::pair<double,double>(min_score,max_score));
                }else{
                    layer_score_map.Put(elements.second.first.layer.id_,std::pair<double,double>(newScore,newScore));
                }
                multiMapScore->Put(common,elements.second);
            }
        }
    }
    return SERVER_SUCCESS;
}

ServerStatus FileSegmentAuditor::CreateOffsetMap(Event event) {
    MPI_Barrier(CONF->server_comm);
    auto file_iter = file_segment_map.find(event.filename);
    if(file_iter == file_segment_map.end()){
        std::string name(event.filename.c_str());
        SegmentMap* mapLayer = new SegmentMap(name + "_SEGEMENT",CONF->is_server,CONF->my_server,CONF->num_servers);
        if(CONF->my_rank_server == 0){
            SegmentScore score;
            score.frequency=0;
            score.time=0;
            PosixFile file;
            file.filename = event.filename;
            file.segment = event.segment;
            file.layer = Layer(event.layer_index);
            mapLayer->Put(event.segment,std::pair<PosixFile,SegmentScore>(file,score));
        }
    }
    MPI_Barrier(CONF->server_comm);
    return ServerStatus::SERVER_SUCCESS;
}

ServerStatus FileSegmentAuditor::SyncCreateOffsetMap(Event event) {
    for(int i=0;i<CONF->num_servers;++i){
        if(i!=CONF->my_server){
            rpc->call(i,FILE_SEGMENT_AUDITOR+"_CreateOffsetMap",event).template as<ServerStatus>();
        }else{
            CreateOffsetMap(event);
        }
    }
    return ServerStatus::SERVER_SUCCESS;
}

std::vector<std::tuple<Segment,SegmentScore, PosixFile>> FileSegmentAuditor::FetchHeatMap(PosixFile file) {
    typedef std::multimap<SegmentScore,std::pair<Segment,PosixFile>> MM;
    typedef std::vector<std::tuple<Segment,SegmentScore, PosixFile>> VT;
    VT vector_tuple=VT();
    MM sorted_map=MM();
    auto iter = file_segment_map.find(file.filename);

    if(iter != file_segment_map.end()){
        SegmentMap* map = iter->second;
        auto allData = map->GetAllData();
        for(auto data:allData){
            sorted_map.emplace(data.second.second,std::pair<Segment,PosixFile>(data.first,data.second.first));
        }
        for(auto iter=sorted_map.begin();iter!=sorted_map.end();++iter){
            vector_tuple.push_back(std::tuple<Segment,SegmentScore, PosixFile>(iter->second.first,iter->first,iter->second.second));
        }
    }
    return vector_tuple;
}

std::map<uint8_t, std::tuple<double, double,double>> FileSegmentAuditor::FetchLayerScores() {
    auto allDatas=layer_score_map.GetAllData();
    std::map<uint8_t, std::tuple<double, double,double>> return_map=std::map<uint8_t, std::tuple<double, double,double>>();
    for(auto data:allDatas){
        Layer layer(data.first);
        double remaining_capcity=layer.capacity_mb_*MB-ioFactory->GetClient(layer.io_client_type)->GetCurrentUsage(layer);
        remaining_capcity=remaining_capcity<0?0:remaining_capcity;
        return_map.emplace(data.first,std::tuple<double, double,double>(data.second.first,data.second.second,remaining_capcity));
    }
    return return_map;
}

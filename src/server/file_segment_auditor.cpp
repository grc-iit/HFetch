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
    }
    return SERVER_SUCCESS;
}

ServerStatus FileSegmentAuditor::UpdateOnMove(PosixFile source, PosixFile destination) {
    auto iter = file_segment_map.find(source.filename);
    if(iter != file_segment_map.end()){
        SegmentMap* multiMapScore = iter->second;
        auto iter = multiMapScore->Get(source.segment);
        auto allDatas = multiMapScore->Contains(source.segment);
        for(auto elements : allDatas){
            multiMapScore->Erase(elements.first);
            /* retain left over scores */
            auto left_overs = elements.first.Substract(source.segment);
            for(auto left_over : left_overs){
                multiMapScore->Put(left_over,elements.second);
            }
            /* update intersected score */
            auto common = source.segment.Intersect(elements.first);
            auto previous_score = elements.second.second.GetScore();
            elements.second.first = destination;
            elements.second.first.segment.start = common.start - source.segment.start;
            elements.second.first.segment.end = common.end - source.segment.start;
            multiMapScore->Put(common,elements.second);
            auto layer_score_iter = layer_score_map.Get(elements.second.first.layer.id_);
            if(layer_score_iter.first){
                auto ret = layer_score_iter.second.equal_range(previous_score);
                auto it=ret.first;
                while( it!=ret.second){
                    if(it->second.first.filename == source.filename && it->second.first.segment == elements.first){
                        it = layer_score_iter.second.erase(it);
                    }else ++it;
                }
                PosixFile sub_buf=elements.second.first;
                PosixFile sub_source=source;
                sub_source.segment = common;
                sub_buf.segment = common;
                layer_score_iter.second.emplace(elements.second.second.GetScore(),std::pair<PosixFile,PosixFile>(sub_source,sub_buf));
                layer_score_map.Put(elements.second.first.layer.id_,layer_score_iter.second);
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
        auto allDatas = multiMapScore->Contains(event.segment);
        for(auto elements : allDatas){
            multiMapScore->Erase(elements.first);
            /* retain left over scores */
            auto left_overs = elements.first.Substract(event.segment);
            for(auto left_over : left_overs){
                multiMapScore->Put(left_over,elements.second);
            }
            /* update intersected score */
            auto common = event.segment.Intersect(elements.first);
            auto previous_score = elements.second.second.GetScore();
            elements.second.second.frequency+=1;
            elements.second.second.lrf+=pow(.5,LAMDA_FOR_SCORE*event.time/1000000.0);
            double newScore = elements.second.second.GetScore();
            printf("File:%s,%ld,%ld Score:%f\n",
                    event.filename.c_str(),
                    elements.second.first.segment.start,
                    elements.second.first.segment.end,
                    newScore);
            multiMapScore->Put(common,elements.second);
            auto layer_score_iter = layer_score_map.Get(elements.second.first.layer.id_);
            if(layer_score_iter.first){
                auto ret = layer_score_iter.second.equal_range(previous_score);
                auto it=ret.first;
                while( it!=ret.second){
                    if(it->second.first.filename == event.filename && it->second.first.segment == elements.first){
                        it = layer_score_iter.second.erase(it);
                    }else ++it;
                }
                PosixFile sub_buf=elements.second.first;
                sub_buf.segment = common;
                PosixFile sub_source;
                sub_source.filename=event.filename;
                sub_source.layer=Layer(event.layer_index);
                sub_source.segment = common;
                layer_score_iter.second.emplace(elements.second.second.GetScore(),std::pair<PosixFile,PosixFile>(sub_source,sub_buf));
                layer_score_map.Put(elements.second.first.layer.id_,layer_score_iter.second);
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
            score.lrf=0;
            PosixFile file;
            file.filename = event.filename;
            file.segment = event.segment;
            file.layer = Layer(event.layer_index);
            mapLayer->Put(event.segment,std::pair<PosixFile,SegmentScore>(file,score));
        }
        file_segment_map.emplace(event.filename,mapLayer);
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

std::map<uint8_t, std::multimap<double,std::pair<PosixFile,PosixFile>,std::greater<double>>> FileSegmentAuditor::FetchLayerScores() {
    auto allDatas=layer_score_map.GetAllData();
    auto return_map=std::map<uint8_t, std::multimap<double,std::pair<PosixFile,PosixFile>,std::greater<double>>>();
    for(auto data:allDatas){
        return_map.emplace(data.first,data.second);
    }
    return return_map;
}

std::vector<std::pair<PosixFile, PosixFile>> FileSegmentAuditor::GetDataLocation(PosixFile file) {
    std::vector<std::pair<PosixFile, PosixFile>> values = std::vector<std::pair<PosixFile, PosixFile>>();
    auto iter = file_segment_map.find(file.filename);
    long original_start=0;
    if(iter != file_segment_map.end()){
        SegmentMap* map = iter->second;
        auto allData = map->Contains(file.segment);
        for(auto data:allData){
            /* update intersected score */
            auto common = file.segment.Intersect(data.first);
            PosixFile source = data.second.first;
            source.segment.start = common.start - data.first.start;
            source.segment.end = common.end - data.first.start;
            PosixFile destination = data.second.first;
            destination.segment.start = original_start;
            destination.segment.end = original_start + common.end - common.start;
            original_start += common.end - common.start;
            values.push_back(std::pair<PosixFile, PosixFile>(source,destination));
        }
    }
    return values;
}

std::vector<std::pair<PosixFile, PosixFile>> FileSegmentAuditor::GetDataLocationServer(PosixFile file,uint16_t server) {
    return rpc->call(server,FILE_SEGMENT_AUDITOR+"_GetDataLocation",file).template as<std::vector<std::pair<PosixFile, PosixFile>>>();;
}

bool FileSegmentAuditor::CheckIfFileActive(PosixFile file) {
    auto iter = file_active_status.Get(file.filename);
    if(iter.first){
        return iter.second != 0;
    }
    return false;
}

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
                multiMapScore->Put(common,elements.second);
            }
        }
    }
    return SERVER_SUCCESS;
}

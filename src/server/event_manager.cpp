//
// Created by hariharan on 3/19/19.
//

#include "event_manager.h"


ServerStatus EventManager::handle(std::vector<Event> events) {
    AutoTrace trace = AutoTrace("EventManager::handle",events);
    auditor->Update(events);
    auto placements = dpe->place(events);
    for(auto placement : placements){
        PosixFile source=std::get<0>(placement);
        PosixFile destination=std::get<1>(placement);
        double score=std::get<2>(placement);
        if(!dataManager->HasCapacity(destination.GetSize(),destination.layer)){
            dataManager->MakeCapacity(destination.GetSize(),score,destination.layer);
        }
        dataManager->Prefetch(source,destination);
        auditor->UpdateOnMove(source,destination);
    }
    for(auto event:events){
        if(event.event_type==EventType::FILE_CLOSE){
            PosixFile file;
            file.filename=event.filename;
            file.segment=event.segment;
            file.layer=Layer(event.layer_index);
            bool isFileActive = auditor->CheckIfFileActive(file);
            if(!isFileActive){
                auto heatMap = auditor->FetchHeatMap(file);
                for(auto segment_tuple:heatMap){
                    PosixFile buf_file=std::get<2>(segment_tuple);
                    if(buf_file.layer!=*Layer::LAST) ioFactory->GetClient(buf_file.layer.io_client_type)->Delete(buf_file);
                }
            }
        }
    }
    return SERVER_SUCCESS;
}

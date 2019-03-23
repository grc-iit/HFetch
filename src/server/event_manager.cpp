//
// Created by hariharan on 3/19/19.
//

#include "event_manager.h"


ServerStatus EventManager::handle(std::vector<Event> events) {
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
    return SERVER_SUCCESS;
}

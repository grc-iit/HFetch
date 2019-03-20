//
// Created by hariharan on 3/19/19.
//

#include "event_manager.h"

ServerStatus EventManager::handle(std::vector<Event> events) {
    auditor->Update(events);
    auto placements = dpe->place(events);
    for(auto placement : placements){
        dataManager->Prefetch(placement.first,placement.second);
        auditor->UpdateOnPrefetch(placement.first,placement.second);
    }
    return SERVER_SUCCESS;
}

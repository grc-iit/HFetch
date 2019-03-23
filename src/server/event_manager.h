//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_EVENT_MANAGER_H
#define HFETCH_EVENT_MANAGER_H


#include <src/common/enumerations.h>
#include <src/common/data_structure.h>
#include <src/common/io_clients/io_client.h>
#include <src/common/io_clients/io_client_factory.h>
#include <src/server/dpe/dpe.h>
#include <src/server/dpe/dpe_factory.h>
#include <src/common/io_clients/data_manager.h>
#include "file_segment_auditor.h"

class EventManager {
    std::shared_ptr<FileSegmentAuditor> auditor;
    std::shared_ptr<DPE> dpe;
    std::shared_ptr<DataManager> dataManager;
    std::shared_ptr<IOClientFactory> ioFactory;
public:
    EventManager(){
        ioFactory = Singleton<IOClientFactory>::GetInstance();
        auditor = Singleton<FileSegmentAuditor>::GetInstance();
        dpe = Singleton<DPEFactory>::GetInstance()->GetEngine(CONF->dpeType);
        dataManager = Singleton<DataManager>::GetInstance();
    }
    ServerStatus handle(std::vector<Event> events);
};


#endif //HFETCH_EVENT_MANAGER_H

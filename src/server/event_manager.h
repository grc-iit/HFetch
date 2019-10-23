/*
 * Copyright (C) 2019  SCS Lab <scs-help@cs.iit.edu>, Hariharan
 * Devarajan <hdevarajan@hawk.it.edu>, Xian-He Sun <sun@iit.edu>
 *
 * This file is part of HFetch
 * 
 * HFetch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
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
        AutoTrace trace = AutoTrace("EventManager");
        ioFactory = Singleton<IOClientFactory>::GetInstance();
        auditor = Singleton<FileSegmentAuditor>::GetInstance();
        dpe = Singleton<DPEFactory>::GetInstance()->GetEngine(CONF->dpeType);
        dataManager = Singleton<DataManager>::GetInstance();
    }
    ServerStatus handle(std::vector<Event> events);
};


#endif //HFETCH_EVENT_MANAGER_H

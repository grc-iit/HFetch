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

#ifndef HFETCH_FILE_CLIENT_H
#define HFETCH_FILE_CLIENT_H


#include <src/common/distributed_ds/hashmap/DistributedHashMap.h>
#include <src/common/configuration_manager.h>
#include "io_client.h"

class SharedFileClient: public IOClient {
    DistributedHashMap<uint8_t,bool> hasChanged;
    DistributedHashMap<uint8_t,double> layerCapacity;
public:
    SharedFileClient():  hasChanged("LAYER_CHANGED",CONF->is_server,CONF->my_server,CONF->num_servers),
                        layerCapacity("LAYER_CAPACITY",CONF->is_server,CONF->my_server,CONF->num_servers){}
    SharedFileClient(std::string name):  hasChanged(name+"_LAYER_CHANGED",CONF->is_server,CONF->my_server,CONF->num_servers),
                         layerCapacity(name+"_LAYER_CAPACITY",CONF->is_server,CONF->my_server,CONF->num_servers){}
    ServerStatus Read(PosixFile &source, PosixFile &destination) override;
    ServerStatus Write(PosixFile &source, PosixFile &destination) override;
    ServerStatus Delete(PosixFile file) override;
    double GetCurrentUsage(Layer layer) override;
};


#endif //HFETCH_FILE_CLIENT_H

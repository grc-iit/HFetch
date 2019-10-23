/*
 * Copyright (C) 2019  SCS Lab <scs-help@cs.iit.edu>, Hariharan
 * Devarajan <hdevarajan@hawk.iit.edu>, Xian-He Sun <sun@iit.edu>
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

#ifndef HFETCH_MEMORY_CLIENT_H
#define HFETCH_MEMORY_CLIENT_H


#include <src/common/distributed_ds/hashmap/DistributedHashMap.h>
#include <boost/interprocess/containers/string.hpp>
#include <src/common/configuration_manager.h>
#include <src/common/data_structure.h>
#include "io_client.h"

namespace bip=boost::interprocess;
typedef bip::allocator<char, bip::managed_shared_memory::segment_manager> CharAllocator;
typedef boost::interprocess::basic_string<char, std::char_traits<char>, CharAllocator> MyShmString;
typedef boost::interprocess::allocator<MyShmString, boost::interprocess::managed_shared_memory::segment_manager> StringAllocator;

class MemoryClient: public IOClient {
private:
    DistributedHashMap<CharStruct,PosixFile> data_map;
    std::shared_ptr<RPC> rpc;
    const std::string MEMORY_CLIENT="MEMORY_CLIENT";
public:
    ~MemoryClient(){
        AutoTrace trace = AutoTrace("~MemoryClient");
        if(CONF->is_server){
            auto datas = data_map.GetAllDataInServer();
            for(auto data:datas){
                bip::shared_memory_object::remove(data.second.filename.c_str());
            }
        }
    }

    MemoryClient():data_map("DATA_MAP",CONF->is_server,CONF->my_server,CONF->num_servers){
        AutoTrace trace = AutoTrace("MemoryClient");
        rpc=Singleton<RPC>::GetInstance("RPC_SERVER_LIST",CONF->is_server,CONF->my_server,CONF->num_servers);
        if(CONF->is_server){
            std::function<ServerStatus(PosixFile&,PosixFile&)> readFunc(std::bind(&MemoryClient::Read, this, std::placeholders::_1, std::placeholders::_2));
            std::function<ServerStatus(PosixFile&,PosixFile&)> writeFunc(std::bind(&MemoryClient::Write, this, std::placeholders::_1, std::placeholders::_2));
            std::function<ServerStatus(PosixFile)> deleteFunc(std::bind(&MemoryClient::Delete, this, std::placeholders::_1));
            rpc->bind(MEMORY_CLIENT+"_Read", readFunc);
            rpc->bind(MEMORY_CLIENT+"_Write", writeFunc);
            rpc->bind(MEMORY_CLIENT+"_Delete", deleteFunc);
        }
    }
    ServerStatus Read(PosixFile &source, PosixFile &destination) override;
    ServerStatus Write(PosixFile &source, PosixFile &destination) override;
    ServerStatus Delete(PosixFile file) override;
    double GetCurrentUsage(Layer layer) override;
};


#endif //HFETCH_MEMORY_CLIENT_H

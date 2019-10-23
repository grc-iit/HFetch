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
// Created by hariharan on 3/26/19.
//

#ifndef HFETCH_SHARED_FILE_CLIENT_H
#define HFETCH_SHARED_FILE_CLIENT_H


#include "shared_file_client.h"

class LocalFileClient : public SharedFileClient{
    std::shared_ptr<RPC> rpc;
    const std::string LOCAL_FILE_CLIENT="LOCAL_FILE_CLIENT";
public:
    LocalFileClient(): SharedFileClient("L") {
        AutoTrace trace = AutoTrace("LocalFileClient");
        rpc=Singleton<RPC>::GetInstance("RPC_SERVER_LIST",CONF->is_server,CONF->my_server,CONF->num_servers);
        if(CONF->is_server){
            std::function<ServerStatus(PosixFile&,PosixFile&)> readFunc(std::bind(&LocalFileClient::Read, this, std::placeholders::_1, std::placeholders::_2));
            std::function<ServerStatus(PosixFile&,PosixFile&)> writeFunc(std::bind(&LocalFileClient::Write, this, std::placeholders::_1, std::placeholders::_2));
            std::function<ServerStatus(PosixFile)> deleteFunc(std::bind(&LocalFileClient::Delete, this, std::placeholders::_1));
            rpc->bind(LOCAL_FILE_CLIENT+"_Read", readFunc);
            rpc->bind(LOCAL_FILE_CLIENT+"_Write", writeFunc);
            rpc->bind(LOCAL_FILE_CLIENT+"_Delete", deleteFunc);
        }
    }

    ServerStatus Read(PosixFile &source, PosixFile &destination) override {
        std::size_t hash_val = std::hash<CharStruct>()(source.filename)%CONF->num_servers;
        if(hash_val == CONF->my_server){
            AutoTrace trace = AutoTrace("LocalFileClient::Read(local)",source,destination);
            return SharedFileClient::Read(source, destination);
        }else{
            AutoTrace trace = AutoTrace("LocalFileClient::Read(remote)",source,destination);
            return rpc->call(hash_val,LOCAL_FILE_CLIENT+"_Read",source,destination).template as<ServerStatus>();
        }

    }

    ServerStatus Write(PosixFile &source, PosixFile &destination) override {
        std::size_t hash_val = std::hash<CharStruct>()(source.filename)%CONF->num_servers;
        if(hash_val == CONF->my_server){
            AutoTrace trace = AutoTrace("LocalFileClient::Write(local)",source,destination);
            return SharedFileClient::Write(source, destination);
        }else{
            AutoTrace trace = AutoTrace("LocalFileClient::Write(remote)",source,destination);
            return rpc->call(hash_val,LOCAL_FILE_CLIENT+"_Write",source,destination).template as<ServerStatus>();
        }
    }

    ServerStatus Delete(PosixFile file) override {
        std::size_t hash_val = std::hash<CharStruct>()(file.filename)%CONF->num_servers;
        if(hash_val == CONF->my_server){
            AutoTrace trace = AutoTrace("LocalFileClient::Delete(local)",file);
            return SharedFileClient::Delete(file);
        }else{
            AutoTrace trace = AutoTrace("LocalFileClient::Delete(remote)",file);
            return rpc->call(hash_val,LOCAL_FILE_CLIENT+"_Delete",file).template as<ServerStatus>();
        }
    }

    double GetCurrentUsage(Layer layer) override {
        return SharedFileClient::GetCurrentUsage(layer);
    }
};


#endif //HFETCH_SHARED_FILE_CLIENT_H

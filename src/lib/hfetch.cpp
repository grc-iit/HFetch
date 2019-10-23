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
// Created by hariharan on 3/18/19.
//

#include <hfetch.h>
#include <mpi.h>
#include <src/common/singleton.h>
#include <src/server/server.h>
#include "metadata_manager.h"

FILE *hfetch::fopen(const char *filename, const char *mode) {
    AutoTrace trace = AutoTrace("hfetch::fopen",filename,mode);
    auto mdm = Singleton<MetadataManager>::GetInstance();
    auto server = Singleton<Server>::GetInstance();
    FILE* fh = std::fopen(filename,mode);
    if(!(strcmp(mode,"a")==0 || strcmp(mode,"a+")==0)) {
        std::fseek(fh, 0L, SEEK_END);
    }
    long file_size = std::ftell(fh);
    if(!(strcmp(mode,"a")==0 || strcmp(mode,"a+")==0)){
        rewind(fh);
    }
    mdm->Update(fh,filename);
    Event event;
    event.filename=CharStruct(basename(filename));
    event.segment.start=0;
    event.segment.end=file_size-1;
    event.layer_index=Layer::LAST->id_;
    event.source=EventSource::APPLICATION;
    event.event_type=EventType::FILE_OPEN;
    server->pushEvents(event);
    return fh;
}

int hfetch::fclose(FILE *fh) {
    AutoTrace trace = AutoTrace("hfetch::fclose");
    auto mdm = Singleton<MetadataManager>::GetInstance();
    auto server = Singleton<Server>::GetInstance();
    auto result=mdm->GetFilename(fh);
    if(result.first){
        std::string filename = result.second;
        std::fseek(fh, 0L, SEEK_END);
        long file_size = std::ftell(fh);
        int r = std::fclose(fh);
        mdm->Delete(fh);
        Event event;
        event.filename=CharStruct(basename(result.second.c_str()));
        event.segment.start=0;
        event.segment.end=file_size-1;
        event.layer_index=Layer::LAST->id_;
        event.source=EventSource::APPLICATION;
        event.event_type=EventType::FILE_CLOSE;
        server->pushEvents(event);
        return r;
    }
    return -1;
}

int hfetch::fseek(FILE *fh, long int offset, int origin) {
    AutoTrace trace = AutoTrace("hfetch::fseek");
    return std::fseek(fh,offset,origin);
}

size_t hfetch::fread(void *ptr, size_t size, size_t count, FILE *fh) {
    auto mdm = Singleton<MetadataManager>::GetInstance();
    auto server = Singleton<Server>::GetInstance();
    auto result=mdm->GetFilename(fh);
    if(result.first){
        long current_offset=std::fseek(fh, 0L, SEEK_CUR);
        AutoTrace trace = AutoTrace("hfetch::fread",current_offset,result.second,size*count);
        std::fseek(fh, 0L, SEEK_END);
        long file_size = std::ftell(fh);
        std::fseek(fh, current_offset, SEEK_SET);
        Event event;
        event.filename=CharStruct(basename(result.second.c_str()));
        event.segment.start=current_offset;
        event.segment.end=current_offset+size*count-1;
        event.layer_index=Layer::LAST->id_;
        event.source=EventSource::APPLICATION;
        event.event_type=EventType::FILE_READ;
        server->pushEvents(event);
        PosixFile file;
        file.filename = CharStruct(basename(result.second.c_str()));
        file.segment = event.segment;
        file.layer = *Layer::LAST;
        auto datas = server->GetDataLocation(file);
        size_t original_offset=0;
        if(datas.size()>0){
            for(auto data:datas){
                CONF->total++;
                if(data.second.GetSize()>0){
                    if(data.first.layer != *Layer::LAST) CONF->hit++;
                    data.second.data.reserve(data.second.GetSize());
                    Singleton<IOClientFactory>::GetInstance()->GetClient(data.first.layer.io_client_type)->Read(data.first,data.second);
                    memcpy((char*)ptr+original_offset,data.second.data.data(),data.second.GetSize());
                    original_offset+=data.second.GetSize();
                }

            }
        }else{
            std::fread(ptr,size,count,fh);
            CONF->total++;
        }

        return size*count;
    }
    return -1;
}

size_t hfetch::fwrite(const void *ptr, size_t size, size_t count, FILE *fh) {
    AutoTrace trace = AutoTrace("hfetch::fwrite",size*count);
    return std::fwrite(ptr,size,count,fh);
}

InputArgs hfetch::MPI_Init(int *argc, char ***argv) {
    PMPI_Init(argc,argv);
    AutoTrace trace = AutoTrace("hfetch::MPI_Init");
    InputArgs args = Server::InitializeClients(*argc,*argv);
    Singleton<MetadataManager>::GetInstance();
    return args;
}

int hfetch::MPI_Finalize() {
    return PMPI_Finalize();
}



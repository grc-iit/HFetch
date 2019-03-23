//
// Created by hariharan on 3/18/19.
//

#include <hfetch.h>
#include <mpi.h>
#include <src/common/singleton.h>
#include <src/server/server.h>
#include "metadata_manager.h"

FILE *hfetch::fopen(const char *filename, const char *mode) {
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
    return std::fseek(fh,offset,origin);
}

size_t hfetch::fread(void *ptr, size_t size, size_t count, FILE *fh) {
    auto mdm = Singleton<MetadataManager>::GetInstance();
    auto server = Singleton<Server>::GetInstance();
    auto result=mdm->GetFilename(fh);
    if(result.first){
        long current_offset=std::fseek(fh, 0L, SEEK_CUR);
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
        for(auto data:datas){
            if(data.first.layer == *Layer::LAST) printf("Read from PFS\n");
            data.second.data=ptr;
            Singleton<IOClientFactory>::GetInstance()->GetClient(data.first.layer.io_client_type)->Read(data.first,data.second);
        }
        return size*count;
    }
    return -1;
}

size_t hfetch::fwrite(const void *ptr, size_t size, size_t count, FILE *fh) {
    return std::fwrite(ptr,size,count,fh);
}

InputArgs hfetch::MPI_Init(int *argc, char ***argv) {
    PMPI_Init(argc,argv);
    InputArgs args = Server::InitializeClients(*argc,*argv);
    Singleton<MetadataManager>::GetInstance();
    return args;
}

int hfetch::MPI_Finalize() {
    return PMPI_Finalize();
}



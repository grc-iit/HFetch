//
// Created by hariharan on 3/19/19.
//

#include "memory_client.h"

ServerStatus MemoryClient::Read(PosixFile source, PosixFile destination) {
    auto iter = data_map.Get(source.filename);
    if(iter.first){
        memcpy(destination.data,iter.second.c_str()+source.segment.start,source.GetSize());
        return SERVER_SUCCESS;
    }
    return SERVER_FAILED;
}

ServerStatus MemoryClient::Write(PosixFile source, PosixFile destination) {
    auto iter = data_map.Get(destination.filename);
    if(iter.first){
        if(iter.second.size() == source.GetSize() && source.segment.start == 0) {
            data_map.Put(destination.filename, bip::string((char*)source.data));
        }else if(iter.second.size() >= source.segment.end){
            char* data = static_cast<char *>(malloc(iter.second.size()));
            memcpy(data,iter.second.c_str(),iter.second.size());
            memcpy(data+source.segment.start, source.data,source.GetSize());
            data_map.Put(destination.filename, bip::string(data));
            free(data);
        }else{
            size_t new_size = iter.second.size() - source.segment.start - 1 + source.GetSize();
            char* data = static_cast<char *>(malloc(new_size));
            memcpy(data,iter.second.c_str(),source.segment.start - 1);
            memcpy(data + source.segment.start, source.data,source.GetSize());
            data_map.Put(destination.filename, bip::string(data));
            free(data);
        }
    }else{
        data_map.Put(destination.filename, bip::string((char*)source.data));
    }
    return SERVER_SUCCESS;
}

ServerStatus MemoryClient::Delete(PosixFile file) {
    auto iter = data_map.Get(file.filename);
    if(iter.first){
        if(iter.second.size() == file.GetSize() && file.segment.start == 0) {
            data_map.Erase(file.filename);
        }else if(iter.second.size() >= file.segment.end){
            char* data = static_cast<char *>(malloc(iter.second.size() - file.GetSize()));
            memcpy(data,iter.second.c_str(),file.segment.start - 1);
            memcpy(data+file.segment.start, iter.second.c_str() + file.segment.end,iter.second.size() - file.segment.end);
            data_map.Put(file.filename, bip::string(data));
            free(data);
        }else{
            if(file.segment.start > 0){
                size_t new_size = file.segment.start - 1;
                char* data = static_cast<char *>(malloc(new_size));
                memcpy(data,iter.second.c_str(),file.segment.start - 1);
                data_map.Put(file.filename, bip::string(data));
                free(data);
            }
        }
    }
    return SERVER_SUCCESS;
}

double MemoryClient::GetCurrentUsage(Layer l) {
    auto datas = data_map.GetAllData();
    double total=0.0;
    for(auto data:datas){
        total+=data.second.size();
    }
    return total;
}

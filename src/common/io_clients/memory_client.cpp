//
// Created by hariharan on 3/19/19.
//

#include "memory_client.h"

ServerStatus MemoryClient::Read(PosixFile &source, PosixFile &destination) {
    std::size_t hash_val = std::hash<CharStruct>()(source.filename)%CONF->num_servers;
    if(hash_val == CONF->my_server){
        AutoTrace trace = AutoTrace("MemoryClient::Read(local)",source,destination);
        auto iter = data_map.Get(source.filename);
        if(iter.first){
            auto shm = new bip::managed_shared_memory(bip::open_only, iter.second.filename.c_str());
            MyShmString *str = shm->find<MyShmString>("myShmString").first;
            destination.data.assign(str->c_str()+source.segment.start,source.GetSize());
            return SERVER_SUCCESS;
        }
    }else{
        AutoTrace trace = AutoTrace("MemoryClient::Read(remote)",source,destination);
        return rpc->call(hash_val,MEMORY_CLIENT+"_Read",source,destination).template as<ServerStatus>();
    }
    return SERVER_FAILED;
}

ServerStatus MemoryClient::Write(PosixFile &source, PosixFile &destination) {
    std::size_t hash_val = std::hash<CharStruct>()(source.filename)%CONF->num_servers;
    if(hash_val == CONF->my_server){
        AutoTrace trace = AutoTrace("MemoryClient::Write(local)",source,destination);
        auto iter = data_map.Get(destination.filename);
        if(iter.first){
            if(iter.second.GetSize() == source.GetSize() && source.segment.start == 0) {
                bip::shared_memory_object::remove(destination.filename.c_str());
                auto shm = new bip::managed_shared_memory(bip::create_only,destination.filename.c_str() , source.GetSize()+1024);
                CharAllocator charallocator(shm->get_segment_manager());
                MyShmString *myShmString = shm->construct<MyShmString>("myShmString")(charallocator);
                myShmString->assign(source.data.data(), source.GetSize());
                PosixFile dummy=destination;
                dummy.data=bip::string();
                data_map.Put(destination.filename, dummy);
            }else if(iter.second.GetSize() >= source.segment.end){
                char* data = static_cast<char *>(malloc(iter.second.GetSize()));
                auto shm = new bip::managed_shared_memory(bip::open_only, iter.second.filename.c_str());
                MyShmString *str = shm->find<MyShmString>("myShmString").first;
                memcpy(data,str->c_str(),str->size());
                memcpy(data+source.segment.start, source.data.data(),source.GetSize());
                str->assign(data, iter.second.GetSize());
                free(data);
            }else{
                size_t new_size = iter.second.GetSize() - source.segment.start + 1 + source.GetSize();
                char* data = static_cast<char *>(malloc(new_size));
                auto shm = new bip::managed_shared_memory(bip::open_only, iter.second.filename.c_str());
                MyShmString *str = shm->find<MyShmString>("myShmString").first;
                memcpy(data,str->c_str(),source.segment.start + 1);
                memcpy(data + source.segment.start, source.data.data(),source.GetSize());
                bip::shared_memory_object::remove(iter.second.filename.c_str());
                shm = new bip::managed_shared_memory(bip::create_only,destination.filename.c_str() , new_size+1024);
                CharAllocator charallocator(shm->get_segment_manager());
                MyShmString *myShmString = shm->construct<MyShmString>("myShmString")(charallocator);
                myShmString->assign(data, new_size);
                iter.second.segment.end=new_size;
                iter.second.data=bip::string();
                data_map.Put(destination.filename, iter.second);
                free(data);
            }
        }else{
            auto shm = new bip::managed_shared_memory(bip::create_only,destination.filename.c_str() , source.GetSize()+1024);
            CharAllocator charallocator(shm->get_segment_manager());
            MyShmString *myShmString = shm->construct<MyShmString>("myShmString")(charallocator);
            myShmString->assign(source.data.data(), source.GetSize());
            PosixFile dummy=destination;
            dummy.data=bip::string();
            data_map.Put(destination.filename, dummy);
        }
        return SERVER_SUCCESS;
    }else{
        AutoTrace trace = AutoTrace("MemoryClient::Write(remote)",source,destination);
        return rpc->call(hash_val,MEMORY_CLIENT+"_Write",source,destination).template as<ServerStatus>();
    }
}

ServerStatus MemoryClient::Delete(PosixFile file) {
    std::size_t hash_val = std::hash<CharStruct>()(file.filename)%CONF->num_servers;
    if(hash_val == CONF->my_server){
        AutoTrace trace = AutoTrace("MemoryClient::Delete(local)",file);
        auto iter = data_map.Get(file.filename);
        if(iter.first){
            if(iter.second.GetSize() == file.GetSize() && file.segment.start == 0) {
                data_map.Erase(file.filename);
                bip::shared_memory_object::remove(iter.second.filename.c_str());
            }else if(iter.second.GetSize() >= file.segment.end){
                char* data = static_cast<char *>(malloc(iter.second.GetSize() - file.GetSize()));
                auto shm = new bip::managed_shared_memory(bip::open_only, iter.second.filename.c_str());
                MyShmString *str = shm->find<MyShmString>("myShmString").first;
                memcpy(data,str->c_str(),file.segment.start - 1);
                memcpy(data+file.segment.start, str->c_str() + file.segment.end,iter.second.GetSize() - file.segment.end);
                iter.second.segment.end=iter.second.GetSize() - file.GetSize();
                data_map.Put(file.filename, iter.second);
                free(data);
            }
        }
        return SERVER_SUCCESS;
    }else{
        AutoTrace trace = AutoTrace("MemoryClient::Delete(remote)",file);
        return rpc->call(hash_val,MEMORY_CLIENT+"_Delete",file).template as<ServerStatus>();
    }

}

double MemoryClient::GetCurrentUsage(Layer layer) {
    AutoTrace trace = AutoTrace("MemoryClient::GetCurrentUsage",layer);
    auto datas = data_map.GetAllData();
    double total=0.0;
    for(auto data:datas){
        total+=data.second.GetSize();
    }
    return total;
}

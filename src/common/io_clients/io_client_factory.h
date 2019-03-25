//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_IO_CLIENT_FACTORY_H
#define HFETCH_IO_CLIENT_FACTORY_H

#include <src/common/singleton.h>
#include "io_client.h"
#include "file_client.h"
#include "memory_client.h"

class IOClientFactory{
public:
    IOClientFactory(){
        AutoTrace trace = AutoTrace("IOClientFactory");
        Singleton<FileClient>::GetInstance();
        Singleton<MemoryClient>::GetInstance();
    }
    std::shared_ptr<IOClient> GetClient(IOClientType type){
        AutoTrace trace = AutoTrace("GetClient",type);
        switch(type){
            case IOClientType::POSIX_FILE:{
                return Singleton<FileClient>::GetInstance();
            }
            case IOClientType::SIMPLE_MEMORY:{
                return Singleton<MemoryClient>::GetInstance();
            }
        }
    }
};
#endif //HFETCH_IO_CLIENT_FACTORY_H

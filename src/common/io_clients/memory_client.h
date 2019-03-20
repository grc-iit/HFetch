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

class MemoryClient: public IOClient {
private:
    DistributedHashMap<CharStruct,bip::string> data_map;
public:
    MemoryClient():data_map("DATA_MAP",CONF->is_server,CONF->my_server,CONF->num_servers){}
    ServerStatus Read(PosixFile source, PosixFile destination) override;
    ServerStatus Write(PosixFile source, PosixFile destination) override;
    ServerStatus Delete(PosixFile file) override;
};


#endif //HFETCH_MEMORY_CLIENT_H

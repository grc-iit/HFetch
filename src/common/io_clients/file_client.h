//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_FILE_CLIENT_H
#define HFETCH_FILE_CLIENT_H


#include <src/common/distributed_ds/hashmap/DistributedHashMap.h>
#include <src/common/configuration_manager.h>
#include "io_client.h"

class FileClient: public IOClient {
    DistributedHashMap<uint8_t,bool> hasChanged;
    DistributedHashMap<uint8_t,double> layerCapacity;
public:
    FileClient():hasChanged("LAYER_CHANGED",CONF->is_server,CONF->my_server,CONF->num_servers),
                 layerCapacity("LAYER_CAPACITY",CONF->is_server,CONF->my_server,CONF->num_servers){}
    ServerStatus Read(PosixFile &source, PosixFile &destination) override;
    ServerStatus Write(PosixFile &source, PosixFile &destination) override;
    ServerStatus Delete(PosixFile file) override;
    double GetCurrentUsage(Layer layer) override;
};


#endif //HFETCH_FILE_CLIENT_H

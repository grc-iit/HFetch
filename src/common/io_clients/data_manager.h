//
// Created by hariharan on 3/20/19.
//

#ifndef HFETCH_DATA_MANAGER_H
#define HFETCH_DATA_MANAGER_H

#include <src/common/enumerations.h>
#include <src/common/data_structure.h>

class DataManager{
private:
    std::shared_ptr<IOClientFactory> ioFactory;
public:
    DataManager(){
        ioFactory = Singleton<IOClientFactory>::GetInstance();
    }
    ServerStatus Move(PosixFile source, PosixFile destination, bool deleteSource = true) {
        void* data = malloc(source.GetSize());
        destination.data = data;
        ServerStatus status;
        status = ioFactory->GetClient(source.layer.io_client_type)->Read(source,destination);
        if(status != SERVER_SUCCESS) return status;
        status = ioFactory->GetClient(destination.layer.io_client_type)->Write(destination,destination);
        if(status != SERVER_SUCCESS) return status;
        free(data);
        if(deleteSource) status = ioFactory->GetClient(source.layer.io_client_type)->Delete(source);
        return status;
    }

    ServerStatus Prefetch(PosixFile source, PosixFile destination) {
        return Move(source,destination,source.layer!=*Layer::LAST);
    }
};
#endif //HFETCH_DATA_MANAGER_H

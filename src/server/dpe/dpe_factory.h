//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_DPE_FACTORY_H
#define HFETCH_DPE_FACTORY_H

#include <src/common/singleton.h>
#include "dpe.h"
#include "max_bw_dpe.h"

class DPEFactory{
public:
    DPEFactory(){}
    std::shared_ptr<DPE> GetEngine(DataPlacementEngineType type){
        AutoTrace trace = AutoTrace("DPEFactory::GetEngine",type);
        switch (type){
            case DataPlacementEngineType::MAX_BW:{
                return Singleton<MaxBandwidthDPE>::GetInstance();
            };
        }
    }
};
#endif //HFETCH_DPE_FACTORY_H

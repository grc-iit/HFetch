//
// Created by hariharan on 3/21/19.
//

#include <src/common/debug.h>
#include "metadata_manager.h"

std::pair<bool,std::string> MetadataManager::GetFilename(FILE *fh) {

    AutoTrace trace = AutoTrace(" MetadataManager::GetFilename");
    auto iter = fp_map.find(fh);
    if(iter!=fp_map.end()){
        return std::pair<bool,std::string>(true,iter->second);
    }else{
        return std::pair<bool,std::string>(false,std::string());
    }
}

ServerStatus MetadataManager::Update(FILE *fh, std::string filename) {
    AutoTrace trace = AutoTrace(" MetadataManager::Update",filename);
    auto iter = fp_map.find(fh);
    if(iter!=fp_map.end()){
        fp_map.erase(iter);
        fp_map.emplace(fh,filename);
    }else{
        fp_map.emplace(fh,filename);
    }
    return SERVER_SUCCESS;
}

ServerStatus MetadataManager::Delete(FILE *fh) {
    AutoTrace trace = AutoTrace(" MetadataManager::Delete");
    auto iter = fp_map.find(fh);
    if(iter!=fp_map.end()){
        fp_map.erase(iter);
    }
    return SERVER_SUCCESS;
}

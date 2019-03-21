//
// Created by hariharan on 3/21/19.
//

#ifndef HFETCH_METADATAMANAGER_H
#define HFETCH_METADATAMANAGER_H


#include <cstdio>
#include <unordered_map>
#include <src/common/enumerations.h>

class MetadataManager {\
private:
    std::unordered_map<FILE*,std::string> fp_map;
public:
    MetadataManager():fp_map(){}
    std::pair<bool,std::string> GetFilename(FILE* fh);
    ServerStatus Update(FILE* fh,std::string);
    ServerStatus Delete(FILE* fh);
};


#endif //HFETCH_METADATAMANAGER_H

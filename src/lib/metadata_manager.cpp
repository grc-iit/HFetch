/*
 * Copyright (C) 2019  SCS Lab <scs-help@cs.iit.edu>, Hariharan
 * Devarajan <hdevarajan@hawk.iit.edu>, Xian-He Sun <sun@iit.edu>
 *
 * This file is part of HFetch
 * 
 * HFetch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
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

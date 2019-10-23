/*
 * Copyright (C) 2019  SCS Lab <scs-help@cs.iit.edu>, Hariharan
 * Devarajan <hdevarajan@hawk.it.edu>, Xian-He Sun <sun@iit.edu>
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

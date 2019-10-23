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
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_IO_CLIENT_FACTORY_H
#define HFETCH_IO_CLIENT_FACTORY_H

#include <src/common/singleton.h>
#include "io_client.h"
#include "shared_file_client.h"
#include "local_file_client.h"
#include "memory_client.h"

class IOClientFactory{
public:
    IOClientFactory(){
        AutoTrace trace = AutoTrace("IOClientFactory");
        Singleton<SharedFileClient>::GetInstance();
        Singleton<LocalFileClient>::GetInstance();
        Singleton<MemoryClient>::GetInstance();
    }
    std::shared_ptr<IOClient> GetClient(IOClientType type){
        AutoTrace trace = AutoTrace("GetClient",type);
        switch(type){
            case IOClientType::SHARED_POSIX_FILE:{
                return Singleton<SharedFileClient>::GetInstance();
            }
            case IOClientType::LOCAL_POSIX_FILE:{
                return Singleton<LocalFileClient>::GetInstance();
            }
            case IOClientType::SIMPLE_MEMORY:{
                return Singleton<MemoryClient>::GetInstance();
            }
        }
    }
};
#endif //HFETCH_IO_CLIENT_FACTORY_H

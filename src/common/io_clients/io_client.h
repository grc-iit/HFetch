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

#ifndef HFETCH_IO_CLIENT_H
#define HFETCH_IO_CLIENT_H

#include <src/common/enumerations.h>
#include <src/common/data_structure.h>

class IOClient{
public:
    IOClient(){}
    virtual ServerStatus Read(PosixFile &source, PosixFile &destination) = 0;
    virtual ServerStatus Write(PosixFile &source, PosixFile &destination) = 0;
    virtual ServerStatus Delete(PosixFile file) = 0;
    virtual double GetCurrentUsage(Layer l) = 0;

};
#endif //HFETCH_IO_CLIENT_H

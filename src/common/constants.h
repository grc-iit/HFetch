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

#include <cstdint>
#include <cstddef>
#include "typedefs.h"
#include "enumerations.h"

#ifndef HFETCH_CONSTANTS_H
#define HFETCH_CONSTANTS_H
const uint16_t RPC_PORT=8080;
const size_t MAX_STRING_LENGTH=256;
const size_t MAX_PREFETCH_EVENTS=1;
const double LAMDA_FOR_SCORE=0.5;
const std::string FILE_SEPARATOR="/";
const size_t MB=1024*1024;
const ScoreType DEFAULT_SCORE_TYPE=ScoreType::LRF_SCORE;
const int SEGMENT_SIZE=16*1024*1024;





#endif //HFETCH_CONSTANTS_H

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

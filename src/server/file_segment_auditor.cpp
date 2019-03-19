//
// Created by hariharan on 3/19/19.
//

#include "file_segment_auditor.h"

ServerStatus FileSegmentAuditor::OnOpen(CharStruct filename, Segment segment) {
    return SERVER_SUCCESS;
}

ServerStatus FileSegmentAuditor::OnClose(CharStruct filename, Segment segment) {
    return SERVER_SUCCESS;
}

ServerStatus FileSegmentAuditor::OnRead(CharStruct filename, Segment segment) {
    return SERVER_SUCCESS;
}

ServerStatus FileSegmentAuditor::OnPrefetch(CharStruct filename, Segment segment, uint8_t layer_index) {
    return SERVER_SUCCESS;
}

ServerStatus FileSegmentAuditor::GetPrefetchData(CharStruct filename, Segment segment) {
    return SERVER_SUCCESS;
}

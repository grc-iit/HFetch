//
// Created by hariharan on 3/19/19.
//
#include "data_structure.h"

/*
 * initializes the static attributes within Layer class.
 */
Layer* Layer::FIRST = nullptr; /* ram */
Layer* Layer::LAST = nullptr;  /* pfs */

/**
 * Outstream conversions
 */
std::ostream &operator<<(std::ostream &os, char const *m) {
    return os << std::string(m);
}
std::ostream &operator<<(std::ostream &os, uint8_t const &m) {
    return os << std::to_string(m);
}
std::ostream &operator<<(std::ostream &os, LayerInfo const &m){
    return os   << "{TYPE:LayerInfo," << "mount_point_:" << m.mount_point_<< ","
                << "capacity_mb_:" << m.capacity_mb_ << ","
                << "bandwidth:" << m.bandwidth << ","
                << "direct_io:" << m.direct_io << ","
                << "is_memory:" << m.is_memory<<"}";
}
std::ostream &operator<<(std::ostream &os, InputArgs const &m){
    os   << "{TYPE:InputArgs," << "num_servers:" << m.num_servers<< ","
                << "max_files:" << m.max_files<< ","
                << "io_size_:" << m.io_size_<< ","
                << "num_workers:" << m.num_workers << ","
                << "layer_count_:" << m.layer_count_<< ","
                << "ranks_per_server_:" << m.ranks_per_server_<< ","
                << "direct_io_:" << m.direct_io_ << ","
                << "iteration_:" << m.iteration_ << ",";
    os          << "layers:[";
    for(int i=0;i<m.layer_count_;++i){
        os      << "layers("<<i<<"):"<<m.layers[i];
    }
    os          << "]";
    os          << "pfs_path:" << m.pfs_path<<"}";
    return os;

}
std::ostream &operator<<(std::ostream &os, CharStruct const &m){
    return os   << "{TYPE:CharStruct," << "value:" << m.c_str()<<"}";
}
std::ostream &operator<<(std::ostream &os, Segment const &m){
    return os   << "{TYPE:Segment," << "start:" << m.start<< ","
                << "end:" << m.end <<"}";
}
std::ostream &operator<<(std::ostream &os, SegmentScore const &m){
    return os   << "{TYPE:SegmentScore," << "type:" << m.type<< ","
                << "frequency:" << m.frequency << ","
                << "lrf:" << m.lrf <<"}";
}
std::ostream &operator<<(std::ostream &os, Event const &m){
    return os   << "{TYPE:Event," << "event_type:" << m.event_type<< ","
                << "layer_index:" << m.layer_index << ","
                << "filename:" << m.filename << ","
                << "segment:" << m.segment << ","
                << "source:" << m.source << ","
                << "time:" << m.time<<"}";
}
std::ostream &operator<<(std::ostream &os, Layer const &m){
    return os   << "{TYPE:Layer," << "id_:" << m.id_ << ","
                << "capacity_mb_:" << m.capacity_mb_ << ","
                << "layer_loc:" << std::string(m.layer_loc.c_str()) << ","
                << "bandwidth_mbps_:" << m.bandwidth_mbps_ << ","
                << "io_client_type:" << m.io_client_type<<"}";
}
std::ostream &operator<<(std::ostream &os, PosixFile const &m){
    return os   << "{TYPE:PosixFile," << "filename:" << m.filename << ","
                << "segment:" << m.segment << ","
                << "layer:" << m.layer << "}";
}



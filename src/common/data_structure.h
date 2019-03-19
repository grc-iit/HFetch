//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_DATA_STRUCTURE_H
#define HFETCH_DATA_STRUCTURE_H

#include <cstdio>
#include <vector>
#include <string>
#include <cstring>
#include <src/common/constants.h>
#include <src/common/enumerations.h>

/* fixed size char for better inter process communications */
typedef struct CharStruct{
private:
    char value[MAX_STRING_LENGTH];
public:
    CharStruct(){}
    CharStruct(std::string data_){
        sprintf(this->value,"%s",data_.c_str());
    }
    CharStruct(char* data_, size_t size){
        sprintf(this->value,"%s",data_);
    }
    const char* c_str() const{
        return value;
    }

    char* data(){
        return value;
    }
    const size_t size() const{
        return strlen(value);
    }
    /* equal operator for comparing two Matrix. */
    bool operator==(const CharStruct &o) const {
        return strcmp(value,o.value)==0;
    }
} CharStruct;

/* represents the file segment data structure and all its required methods */
typedef struct Segment{
    size_t start;
    size_t end;
    Segment(size_t start_, size_t end_):start(start_),end(end_){} /* Parameterized constructor */
    Segment(const Segment &other) : start(other.start),end(other.end) {} /* copy constructor */
    Segment(Segment &&other) : start(other.start),end(other.end) {} /* move constructor*/
    /* Assignment Operator */
    Segment &operator=(const Segment &other) {
        start=other.start;
        end=other.end;
        return *this;
    }

    /* less than operator for comparing two Segment. */
    bool operator<(const Segment &o) const {
        return o.start > start;
    }

    /* greater than operator for comparing two Segment. */
    bool operator>(const Segment &o) const {
        return o < *this;
    }

    /* equal operator for comparing two Segment. */
    bool operator==(const Segment &o) const {
        return start == o.start && end == o.end;
    }

    /* not equal operator for comparing two Segment. */
    bool operator!=(const Segment &o) const {
        return !(*this == o);
    }


    /**
     * Methods
     */
    /**
     * Checks if the given current Segment contains the given Segment.
     *
     * @param o, given Segment
     * @return true if it contains else false.
     */
    bool Contains(const Segment &o) const {
        return (start <= o.start && end >= o.end) ||
                (start <= o.end && end >= o.end);
    }

    /**
     * This method calculates A - B of Segment. A is this Segment
     * @param o, B Segment
     * @return vectors of Segment which are left over from the substraction.
     */
    std::vector<Segment> Substract(const Segment &o) const {
        std::vector<Segment> left_over = std::vector<Segment>();
        Segment myinstance(*this);
        if (start < o.start && end >= o.end) {
            Segment new_segment(myinstance);
            new_segment.end = o.start - 1;
            left_over.push_back(new_segment);
            myinstance.start = o.start;
        }
        if (start <= o.end && end > o.end) {
            Segment new_segment(myinstance);
            new_segment.start = o.end + 1;
            left_over.push_back(new_segment);
            myinstance.end = o.end;
        }
        return left_over;
    }

    /**
     * Calculates the intersection of two Segment.
     * @param o, is the given Segment
     * @return intersection of two Segment.
     */
    Segment Intersect(Segment &o) const {
        Segment intersection(*this);
        if (start < o.start) {
            intersection.start = o.start;
        } else {
            intersection.start = start;
        }
        if (end < o.end) {
            intersection.end = end;
        } else {
            intersection.end = o.end;
        }
        return intersection;
    }

}Segment;

/* represents the segment score */

typedef struct SegmentScore{
    size_t frequency;
    HTime time;
    SegmentScore(size_t frequency_,HTime time_):frequency(frequency_),time(time_){} /* Parameterized constructor */
    SegmentScore(const SegmentScore &other) : frequency(other.frequency),time(other.time){} /* copy constructor */
    SegmentScore(SegmentScore &&other) : frequency(other.frequency),time(other.time){} /* move constructor*/
    /* Assignment Operator */
    SegmentScore &operator=(const SegmentScore &other) {
        frequency=other.frequency;
        time=other.time;
        return *this;
    }
} SegmentScore;

/* represents an event in the system */
typedef struct Event{
    CharStruct filename;
    Segment segment;
    EventType event_type;
    Event(CharStruct filename_, Segment segment_,EventType event_type_):filename(filename_),segment(segment_),event_type(event_type_){} /* Parameterized constructor */
    Event(const Event &other) : filename(other.filename),segment(other.segment),event_type(other.event_type) {} /* copy constructor */
    Event(Event &&other) : filename(other.filename),segment(other.segment),event_type(other.event_type) {} /* move constructor*/
    /* Assignment Operator */
    Event &operator=(const Event &other) {
        filename=other.filename;
        segment=other.segment;
        event_type=other.event_type;
        return *this;
    }
} Event;


/**
 * This class represents the various hierarchical layers supported hfetch platform.
 */
class Layer {
public:
    /**
     * Attributes
     */
    static Layer *FIRST; /* define enum for First layer */
    static Layer *LAST; /* define enum for Last Layer */
    uint8_t id_; /* id of the layer. */
    float capacity_mb_; /* capacity in Megabytes of the layer. */
    CharStruct layer_loc; /* location of the layer. */
    float bandwidth_mbps_; /* bandwidth in Megabytes per second. */
    Layer *next; /* pointer to next layer. */
    Layer *previous; /* pointer to previous layer. */
    bool direct_io;
    IOClientType io_client_type; /* Type of IO Client to use for the layer*/
    /**
     * Constructors
     */
    Layer():layer_loc() {} /* default constructor */
    Layer(const Layer &other) :
            id_(other.id_),
            capacity_mb_(other.capacity_mb_),
            layer_loc(other.layer_loc),
            bandwidth_mbps_(other.bandwidth_mbps_),
            next(other.next),
            previous(other.previous),
            direct_io(other.direct_io),
            io_client_type(other.io_client_type) {} /* copy constructor */
    /* parameterized constructor */
    Layer(uint8_t id){
        Layer* current=Layer::FIRST;
        while(current != nullptr){
            if(current->id_ == id){
                id_=current->id_;
                capacity_mb_=current->capacity_mb_;
                layer_loc=current->layer_loc;
                next=current->next;
                previous=current->previous;
                bandwidth_mbps_=current->bandwidth_mbps_;
                io_client_type=current->io_client_type;
                break;
            }
            current=current->next;
        }
    }
    Layer(Layer *next,
          Layer *previous,
          bool is_present,
          uint8_t id,
          float capacity,
          CharStruct layer_loc,
          float bandwidth,
          IOClientType io_client_type) :
            id_(id),
            capacity_mb_(capacity),
            layer_loc(layer_loc),
            next(next),
            previous(previous),
            bandwidth_mbps_(bandwidth),
            io_client_type(io_client_type) {
    }

    /**
     * Operators
     */
    /* equal operations for two layers. */
    bool operator==(const Layer &o) const {
        return id_ == o.id_;
    }

    /* not-equal operations for two layers. */
    bool operator!=(const Layer &o) const {
        return id_ != o.id_;
    }


};

#endif //HFETCH_DATA_STRUCTURE_H

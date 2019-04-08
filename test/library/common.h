//
// Created by manih on 4/7/2019.
//

#ifndef HFETCH_COMMON_H
#define HFETCH_COMMON_H
#include <cstdio>
#include <src/common/data_structure.h>
#include <src/common/distributed_ds/hashmap/DistributedHashMap.h>
#include <src/common/distributed_ds/queue/DistributedMessageQueue.h>
#include <src/common/distributed_ds/sequencer/global_sequence.h>
#include <src/common/macros.h>
#include <src/common/configuration_manager.h>
typedef struct EventTest{
    CharStruct filename;
    long offset;
    long size;
    long time;
    EventTest():filename(),offset(),size(),time(){}
    EventTest(const EventTest &other) : filename(other.filename),offset(other.offset),size(other.size),time(other.time) {} /* copy constructor */
    EventTest(EventTest &&other) : filename(other.filename),offset(other.offset),size(other.size),time(other.time) {} /* move constructor*/
    /* Assignment Operator */
    EventTest &operator=(const EventTest &other) {
        filename=other.filename;
        offset=other.offset;
        size=other.size;
        time=other.time;
        return *this;
    }
} EventTest;
namespace clmdep_msgpack {
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
        namespace adaptor {
            namespace mv1=clmdep_msgpack::v1;
            template<>
            struct convert<EventTest> {
                mv1::object const& operator()(mv1::object const& o, EventTest& input) const {
                    input.filename = o.via.array.ptr[0].as<CharStruct>();
                    input.offset = o.via.array.ptr[1].as<long>();
                    input.size = o.via.array.ptr[2].as<long>();
                    input.time = o.via.array.ptr[3].as<long>();
                    return o;
                }
            };

            template<>
            struct pack<EventTest> {
                template <typename Stream>
                packer<Stream>& operator()(mv1::packer<Stream>& o, EventTest const& input) const {
                    // packing member variables as an array.
                    o.pack_array(4);
                    o.pack(input.filename);
                    o.pack(input.offset);
                    o.pack(input.size);
                    o.pack(input.time);
                    return o;
                }
            };

            template <>
            struct object_with_zone<EventTest> {
                void operator()(mv1::object::with_zone& o, EventTest const& input) const {
                    o.type = type::ARRAY;
                    o.via.array.size = 4;
                    o.via.array.ptr = static_cast<clmdep_msgpack::object*>(o.zone.allocate_align(sizeof(mv1::object) * o.via.array.size, MSGPACK_ZONE_ALIGNOF(mv1::object)));
                    o.via.array.ptr[0] = mv1::object(input.filename, o.zone);
                    o.via.array.ptr[1] = mv1::object(input.offset, o.zone);
                    o.via.array.ptr[2] = mv1::object(input.size, o.zone);
                    o.via.array.ptr[3] = mv1::object(input.time, o.zone);
                }
            };
        }
    }
}
class Server{
private:
    DistributedMessageQueue<EventTest> messageQueue;
    DistributedMessageQueue<uint64_t> processedEvents;
    DistributedHashMap<uint64_t,EventTest> processedMap;
    GlobalSequence sequence;
    size_t numDaemons, numEngines;
    std::thread* daemons,*engines;
    std::promise<void>* daemonSignals, *engineSignals;

    void RunDaemons(std::future<void> futureObj,int index){
        Timer pop_time;
        Timer put_time;
        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
            if(messageQueue.Size(CONF->my_server)>0){
                pop_time.resumeTime();
                auto result = messageQueue.Pop(CONF->my_server);
                pop_time.pauseTime();
                if(result.first){
                    uint64_t val=sequence.GetNextSequenceServer(0);
                    processedEvents.Push(val,CONF->my_server);
                    put_time.resumeTime();
                    processedMap.Put(val,result.second);
                    put_time.pauseTime();
                }
            }
        }
        printf("Daemon %d,pop time,%f,put time,%f\n",index,pop_time.getTimeElapsed(),put_time.getTimeElapsed());
    }
    void RunEngines(std::future<void> futureObj,int index){
        Timer engine_time;
        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
            auto result = processedEvents.Pop(CONF->my_server);
            if(result.first){
                engine_time.resumeTime();
                processedMap.Get(result.second);
                engine_time.pauseTime();
            }
        }
        printf("Engine %d,get time,%f\n",index,engine_time.getTimeElapsed());
    }
public:
    ~Server(){
        if(CONF->is_server){
            /*delete(daemons);
            delete(daemonSignals);
            delete(engines);
            delete(engineSignals);*/
        }
    }
    Server():   messageQueue("MESSAGE_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers),
                processedEvents("PROCESSED_EVENTS_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers),
                processedMap("PROCESSED_EVENTS_MAP",CONF->is_server,CONF->my_server,CONF->num_servers),
                sequence("EventSequence",CONF->is_server,CONF->my_server,CONF->num_servers),
                numDaemons(1),numEngines(1){
        if(CONF->is_server){
            daemons = new std::thread[numDaemons];
            daemonSignals = new std::promise<void>[numDaemons];
            engines = new std::thread[numEngines];
            engineSignals = new std::promise<void>[numEngines];
        }
    }
    Server(size_t num_daemons_,size_t num_engines_):messageQueue("MESSAGE_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers),
                                                    processedEvents("PROCESSED_EVENTS_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers),
                                                    processedMap("PROCESSED_EVENTS_MAP",CONF->is_server,CONF->my_server,CONF->num_servers),
                                                    sequence("EventSequence",CONF->is_server,CONF->my_server,CONF->num_servers),
                                                    numDaemons(num_daemons_),numEngines(num_engines_){
        daemons = new std::thread[numDaemons];
        daemonSignals = new std::promise<void>[numDaemons];
        engines = new std::thread[numEngines];
        engineSignals = new std::promise<void>[numEngines];
    }
    void Run(){
        for(int i=0;i<numEngines;++i){
            std::future<void> futureObj = engineSignals[i].get_future();
            engines[i] = std::thread(&Server::RunEngines, this, std::move(futureObj),i);
        }
        for(int i=0;i<numDaemons;++i){
            std::future<void> futureObj = daemonSignals[i].get_future();
            daemons[i] = std::thread(&Server::RunDaemons, this, std::move(futureObj),i);
        }
    }
    void Stop(){
        size_t size = messageQueue.Size(CONF->my_server);

        int count=0;
        while(size > 0){
            size = messageQueue.Size(CONF->my_server);
            if(count%100000==0) printf("MessageQueue Size: %d\n",size);
            count++;
        }
        count=0;
        size = processedEvents.Size(CONF->my_server);
        while(size > 0){
            size = processedEvents.Size(CONF->my_server);
            if(count%100000==0) printf("Processed Event Size: %d\n",size);
            count++;
        }
        for(int i=0;i<numDaemons;++i){
            daemonSignals[i].set_value();
            if(daemons[i].joinable()) daemons[i].join();
        }
        for(int i=0;i<numEngines;++i){
            engineSignals[i].set_value();
            if(engines[i].joinable()) engines[i].join();
        }

    }
    void PushEvent(EventTest event){
        messageQueue.Push(event,CONF->my_server);
    }

};
struct Input{
    size_t num_daemons;
    size_t num_engines;
    size_t num_events;
    int num_servers;
};

inline Input ParseArgs(int argc,char* argv[]){
    Input args;
    args.num_daemons=1;
    args.num_engines=1;
    args.num_events=10000;
    args.num_servers=1;
    int opt;
    /* a:c:d:f:i:l:m:n:p:r:s:w: */
    optind=1;
    while ((opt = getopt (argc, argv, "d:p:e:s:")) != -1)
    {
        switch (opt)
        {
            case 'd':{
                args.num_daemons= atoi(optarg);
                break;
            }
            case 's':{
                args.num_servers= atoi(optarg);
                break;
            }
            case 'p':{
                args.num_engines= atoi(optarg);
                break;
            }
            case 'e':{
                args.num_events= atoi(optarg);
                break;
            }
            default: {}
        }
    }
    return args;
}
#endif //HFETCH_COMMON_H

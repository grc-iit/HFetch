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
        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
            auto result = messageQueue.Pop(CONF->my_server);
            if(result.first){
                uint64_t val=sequence.GetNextSequenceServer(0);
                processedEvents.Push(val,CONF->my_server);
                processedMap.Put(val,result.second);
            }
        }
    }
    void RunEngines(std::future<void> futureObj,int index){
        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
            auto result = processedEvents.Pop(CONF->my_server);
            if(result.first){
                processedMap.Get(result.second);
            }
        }
    }
public:
    ~Server(){
        Stop();
        delete(daemons);
        delete(daemonSignals);
        delete(engines);
        delete(engineSignals);
    }
    Server():messageQueue("MESSAGE_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers),
                                                    processedEvents("PROCESSED_EVENTS_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers),
                                                    processedMap("PROCESSED_EVENTS_MAP",CONF->is_server,CONF->my_server,CONF->num_servers),
                                                    sequence("EventSequence",CONF->is_server,CONF->my_server,CONF->num_servers)
    num_daemons(1),num_engines(1){
        daemons = new std::thread[numDaemons];
        daemonSignals = new std::promise<void>[numDaemons];
        engines = new std::thread[num_engines];
        engineSignals = new std::promise<void>[num_engines];
    }
    Server(size_t num_daemons_,size_t num_engines_):messageQueue("MESSAGE_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers),
                                                    processedEvents("PROCESSED_EVENTS_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers),
                                                    processedMap("PROCESSED_EVENTS_MAP",CONF->is_server,CONF->my_server,CONF->num_servers),
                                                    sequence("EventSequence",CONF->is_server,CONF->my_server,CONF->num_servers)
    num_daemons(num_daemons_),num_engines(num_engines_){
        daemons = new std::thread[numDaemons];
        daemonSignals = new std::promise<void>[numDaemons];
        engines = new std::thread[num_engines];
        engineSignals = new std::promise<void>[num_engines];
    }
    void Run(){
        for(int i=0;i<num_engines;++i){
            std::future<void> futureObj = engineSignals[i].get_future();
            engines[i] = std::thread(&Server::RunEngines, this, std::move(futureObj),i);
        }
        for(int i=0;i<num_engines;++i){
            std::future<void> futureObj = daemonSignals[i].get_future();
            engines[i] = std::thread(&Server::RunDaemons, this, std::move(futureObj),i);
        }
    }
    void Stop(){
        size = messageQueue.Size(CONF->my_server);
        while(size > 0) size = messageQueue.Size(CONF->my_server);
        size_t size = processedEvents.Size(CONF->my_server);
        while(size > 0) size = processedEvents.Size(CONF->my_server);
        for(int i=0;i<numDaemons;++i){
            daemonSignals[i].set_value();
            if(daemons[i].joinable()) daemons[i].join();
        }
        for(int i=0;i<num_engines;++i){
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
};

inline ReaderInput ParseArgs(int argc,char* argv[]){
    ReaderInput args;
    args.num_daemons=1;
    args.num_engines=1;
    args.num_events=10000;
    int opt;
    /* a:c:d:f:i:l:m:n:p:r:s:w: */
    optind=1;
    while ((opt = getopt (argc, argv, "d:p:e:")) != -1)
    {
        switch (opt)
        {
            case 'd':{
                args.num_daemons= atoi(optarg);
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

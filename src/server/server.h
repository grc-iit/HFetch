//
// Created by hariharan on 3/19/19.
//

#ifndef HFETCH_SERVER_H
#define HFETCH_SERVER_H


#include <src/common/distributed_ds/communication/rpc_lib.h>
#include <src/common/singleton.h>
#include <src/common/macros.h>
#include <src/common/configuration_manager.h>
#include <src/common/enumerations.h>
#include <src/common/distributed_ds/queue/DistributedMessageQueue.h>
#include <src/common/distributed_ds/clock/global_clock.h>
#include <src/server/event_manager.h>
#include <src/server/hardware_monitor.h>
#include <src/common/util.h>

class Server {
    std::shared_ptr<RPC> rpc;
    std::shared_ptr<EventManager> eventManager;
    std::shared_ptr<FileSegmentAuditor> auditor;
    std::shared_ptr<HardwareMonitor> hardwareMonitor;
    GlobalClock clock;

    size_t num_workers;
    std::thread* client_server_workers,*monitor_server_workers;
    std::promise<void>* client_server_exit_signal,*monitor_server_exit_signal;
    DistributedMessageQueue<Event> app_event_queue;

    ServerStatus runClientServerInternal(std::future<void> futureObj){
        std::string name="client_thread";
        pthread_setname_np(pthread_self(), name.c_str());
        std::vector<Event> events=std::vector<Event>();
        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
            AutoTrace trace = AutoTrace("Server::runClientServerInternal");
            app_event_queue.WaitForElement(CONF->my_server);
            auto result = app_event_queue.Pop(CONF->my_server);
            if(result.first){
                events.push_back(result.second);
            }
            if(events.size() > 0 && (events.size() >= MAX_PREFETCH_EVENTS)){
                eventManager->handle(events);
                events.clear();
            }
        }
        return ServerStatus::SERVER_SUCCESS;
    }

    ServerStatus runMonitorServerInternal(std::future<void> futureObj){
        /*std::string name="monitor_thread";
        pthread_setname_np(pthread_self(), name.c_str());
        hardwareMonitor->AsyncMonitor();
        std::vector<Event> total_events=std::vector<Event>();
        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
            auto events = hardwareMonitor->FetchEvents();
            if(events.size() > 0) total_events.insert(total_events.begin(),events.begin(),events.end());
            if(events.size() > 0 && (events.size() >= MAX_PREFETCH_EVENTS)){
                eventManager->handle(events);
                events.clear();
            }
        }
        hardwareMonitor->Stop();*/
        return ServerStatus::SERVER_SUCCESS;
    }
public:
    static InputArgs InitializeServer(int argc, char *argv[]){
        AutoTrace trace = AutoTrace("Server::InitializeServer");
        InputArgs args = parse_opts(argc,argv);
        CONF;
        CONF->BuildLayers(args.layers,args.layer_count_);
        CONF->max_num_files=args.max_files;
        CONF->num_workers=args.num_workers;
        CONF->is_server=true;
        CONF->num_servers=CONF->comm_size;
        CONF->UpdateServerComm();
        CONF->my_server=CONF->my_rank_server;
        return args;

    }

    static InputArgs InitializeClients(int argc, char *argv[]){
        AutoTrace trace = AutoTrace("Server::InitializeClients");
        InputArgs args = parse_opts(argc,argv);
        CONF;
        CONF->BuildLayers(args.layers,args.layer_count_);
        CONF->max_num_files=args.max_files;
        CONF->num_workers=args.num_workers;
        CONF->is_server=false;
        CONF->num_servers=args.num_servers;
        CONF->my_server=CONF->my_rank_world/args.num_servers;
        CONF->UpdateServerComm();
        Singleton<Server>::GetInstance();
        Singleton<IOClientFactory>::GetInstance();
        return args;
    }

    Server(size_t num_workers_=1):num_workers(num_workers_),
    app_event_queue("APPLICATION_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers),
    clock("GLOBAL_CLOCK",CONF->is_server,CONF->my_server,CONF->num_servers){
        AutoTrace trace = AutoTrace("Server",num_workers_);
        rpc=Singleton<RPC>::GetInstance("RPC_SERVER_LIST",CONF->is_server,CONF->my_server,CONF->comm_size);
        if(CONF->is_server){
            rpc->run(8);
            eventManager = Singleton<EventManager>::GetInstance();
            hardwareMonitor = Singleton<HardwareMonitor>::GetInstance();
        }
        auditor = Singleton<FileSegmentAuditor>::GetInstance();
        if(CONF->is_server) {

        }
    }

    ServerStatus async_run(size_t numWorker=1){
        AutoTrace trace = AutoTrace("Server::async_run",numWorker);
        num_workers=numWorker;
        if(numWorker > 0){
            client_server_workers=new std::thread[numWorker];
            client_server_exit_signal=new std::promise<void>[numWorker];
            monitor_server_workers=new std::thread[numWorker];
            monitor_server_exit_signal=new std::promise<void>[numWorker];
            for(int i=0;i<numWorker;++i) {
                std::future<void> futureObj = client_server_exit_signal[i].get_future();
                client_server_workers[i]=std::thread (&Server::runClientServerInternal, this, std::move(futureObj));
            /*    std::future<void> futureObj2 = monitor_server_exit_signal[i].get_future();
                monitor_server_workers[i]=std::thread (&Server::runMonitorServerInternal, this, std::move(futureObj2));
            */}
        }
    }

    std::vector<std::pair<PosixFile,PosixFile>> GetDataLocation(PosixFile file){
        AutoTrace trace = AutoTrace("Server::GetDataLocation",file);
        return auditor->GetDataLocation(file);
    }

    ServerStatus pushEvents(Event event){
        AutoTrace trace = AutoTrace("Server::pushEvents",event);
        event.time = clock.GetTimeServer(CONF->my_server);
        app_event_queue.Push(event,CONF->my_server);
    }

    void stop(){
        AutoTrace trace = AutoTrace("Server::stop");
        for (int i = 0; i < num_workers; ++i) {
            /* Issue server kill signals */
            /*monitor_server_exit_signal[i].set_value();*/
            client_server_exit_signal[i].set_value();
            /* wait for servers to end */
            /*if(monitor_server_workers[i].joinable()) monitor_server_workers[i].join();*/
            if(client_server_workers[i].joinable()) client_server_workers[i].join();
        }
    }

};


#endif //HFETCH_SERVER_H

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
#include <src/server/event_manager.h>
#include <src/server/hardware_monitor.h>

class Server {

    std::shared_ptr<RPC> rpc;
    std::shared_ptr<EventManager> eventManager;
    std::shared_ptr<HardwareMonitor> hardwareMonitor;

    size_t num_workers;
    std::thread* client_server_workers,*monitor_server_workers;
    std::promise<void>* client_server_exit_signal,*monitor_server_exit_signal;
    DistributedMessageQueue<Event> event_queue;

    ServerStatus runClientServerInternal(std::future<void> futureObj){
        std::vector<Event> events=std::vector<Event>();
        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
            auto result = event_queue.Pop(CONF->my_server);
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
        hardwareMonitor->Stop();
        return ServerStatus::SERVER_SUCCESS;
    }
public:
    Server(size_t num_workers_=1):num_workers(num_workers_),event_queue("APPLICATION_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers){
        rpc=Singleton<RPC>::GetInstance("RPC_SERVER_LIST",CONF->is_server,CONF->my_server,CONF->comm_size);
        eventManager = Singleton<EventManager>::GetInstance();
        hardwareMonitor = Singleton<HardwareMonitor>::GetInstance();
    }
    ServerStatus async_run(size_t numWorker=1){
        num_workers=numWorker;
        if(numWorker > 0){
            client_server_workers=new std::thread[numWorker];
            client_server_exit_signal=new std::promise<void>[numWorker];
            monitor_server_workers=new std::thread[numWorker];
            monitor_server_exit_signal=new std::promise<void>[numWorker];
            for(int i=0;i<numWorker;++i) {
                std::future<void> futureObj = client_server_exit_signal[i].get_future();
                client_server_workers[i]=std::thread (&Server::runClientServerInternal, this, std::move(futureObj));
                std::future<void> futureObj2 = monitor_server_exit_signal[i].get_future();
                monitor_server_workers[i]=std::thread (&Server::runMonitorServerInternal, this, std::move(futureObj));
            }
        }
    }
    void stop(){
        for (int i = 0; i < num_workers; ++i) {
            /* Issue server kill signals */
            monitor_server_exit_signal[i].set_value();
            client_server_exit_signal[i].set_value();
            /* wait for servers to end */
            monitor_server_workers[i].join();
            client_server_workers[i].join();
        }
    }

};


#endif //HFETCH_SERVER_H

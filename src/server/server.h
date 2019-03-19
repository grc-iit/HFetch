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

class Server {
    size_t num_workers;
    std::shared_ptr<RPC> rpc;
    std::thread* client_server_workers,*monitor_server_workers;
    std::promise<void>* client_server_exit_signal,*monitor_server_exit_signal;
    DistributedMessageQueue<Event> event_queue;
    ServerStatus runClientServerInternal(std::future<void> futureObj){
        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
        }
        return ServerStatus::SERVER_SUCCESS;
    }
    ServerStatus runMonitorServerInternal(std::future<void> futureObj){
        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
        }
        return ServerStatus::SERVER_SUCCESS;
    }
public:
    Server(size_t num_workers_=1):num_workers(num_workers_),event_queue("APPLICATION_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers){
        rpc=Singleton<RPC>::GetInstance("RPC_SERVER_LIST",CONF->is_server,CONF->my_rank,CONF->comm_size);
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

/*
 * Copyright (C) 2019  SCS Lab <scs-help@cs.iit.edu>, Hariharan
 * Devarajan <hdevarajan@hawk.it.edu>, Xian-He Sun <sun@iit.edu>
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

#ifndef HFETCH_HARDWAREMONITOR_H
#define HFETCH_HARDWAREMONITOR_H


#include <src/common/enumerations.h>
#include <src/common/data_structure.h>

#include <sys/inotify.h>
#include <thread>
#include <future>
#include <zconf.h>
#include <src/common/distributed_ds/queue/DistributedMessageQueue.h>
#include <src/common/configuration_manager.h>
#include <src/common/macros.h>

#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 16 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/
class HardwareMonitor {
    /* file handler per layer */
    int num_monitors;
    std::thread* monitor_threads;
    std::promise<void>* monitor__exit_signal;
    DistributedMessageQueue<Event> event_queue;
    ServerStatus AsyncMonitorInternal(std::future<void> futureObj, const Layer* current){
        pthread_setname_np(pthread_self(), std::to_string(current->id_).c_str());
        int length, i = 0,fd,wd;
        char buffer[BUF_LEN];
        uint32_t watch_flags = IN_ALL_EVENTS;
        inotify_add_watch(fd, current->layer_loc.c_str(), watch_flags);
        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
            /*i = 0;
            length = static_cast<int>(read(fd, buffer, BUF_LEN ));

            if ( length < 0 ) {
                perror( "read" );
            }
            while ( i < length ) {
                struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
                if ( event->len ) {
                    if ( event->mask & IN_OPEN) {
                        if (!(event->mask & IN_ISDIR)){
                            printf( "The file %s was Created with WD %d\n", event->name, event->wd );
                            Event openEvent;
                            openEvent.filename=CharStruct(event->name);
                        }
                    }
                    else if ( event->mask & IN_CLOSE_NOWRITE) {
                        if (!(event->mask & IN_ISDIR)){
                            printf( "The file %s was Created with WD %d\n", event->name, event->wd );
                            Event openEvent;
                            openEvent.filename=CharStruct(event->name);
                        }
                    }
                    else if ( event->mask & IN_ACCESS) {
                        if (!(event->mask & IN_ISDIR)){
                            printf( "The file %s was Created with WD %d\n", event->name, event->wd );
                            Event openEvent;
                            openEvent.filename=CharStruct(event->name);
                        }
                    }else{
                        printf( "Unknown Event created The file %s was Created with WD %d\n", event->name, event->wd );
                    }
                    i += EVENT_SIZE + event->len;
                }
            }*/
        }
        /* Clean up*/
        inotify_rm_watch( fd, wd );
        close(fd);
        return ServerStatus::SERVER_SUCCESS;
    }
public:
    HardwareMonitor():num_monitors(0),event_queue("HARDWARE_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers){

        AutoTrace trace = AutoTrace("HardwareMonitor");
        Layer* current=Layer::FIRST;
        while(current != nullptr){
            if(current->io_client_type == IOClientType::LOCAL_POSIX_FILE){
                num_monitors++;
            }
            if(current->io_client_type == IOClientType::SHARED_POSIX_FILE && CONF->my_rank_server ==0){
                num_monitors++;
            }
            current=current->next;
        }
    }
    ServerStatus AsyncMonitor(){
        AutoTrace trace = AutoTrace("HardwareMonitor::AsyncMonitor");
        monitor_threads = new std::thread[num_monitors];
        monitor__exit_signal = new std::promise<void>[num_monitors];
        Layer* current=Layer::FIRST;
        int i = 0;
        while(current != nullptr){
            if(current->io_client_type == IOClientType::LOCAL_POSIX_FILE){
                std::future<void> futureObj = monitor__exit_signal[i].get_future();
                monitor_threads[i]=std::thread (&HardwareMonitor::AsyncMonitorInternal, this, std::move(futureObj),current);
                ++i;
            }
            if(current->io_client_type == IOClientType::SHARED_POSIX_FILE && CONF->my_rank_server == 0){
                std::future<void> futureObj = monitor__exit_signal[i].get_future();
                monitor_threads[i]=std::thread (&HardwareMonitor::AsyncMonitorInternal, this, std::move(futureObj),current);
                ++i;
            }
            current=current->next;

        }

        return ServerStatus::SERVER_SUCCESS;
    }
    std::vector<Event> FetchEvents();
    void Stop(){
        AutoTrace trace = AutoTrace("HardwareMonitor::Stop");
        for (int i = 0; i < num_monitors; ++i) {
            /* Issue server kill signals */
            monitor__exit_signal[i].set_value();
            /* wait for servers to end */
            monitor_threads[i].join();
        }
    }
};


#endif //HFETCH_HARDWAREMONITOR_H

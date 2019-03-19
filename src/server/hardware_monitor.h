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
        int length, i = 0,fd,wd;
        char buffer[BUF_LEN];
        uint32_t watch_flags = IN_CREATE | IN_MODIFY | IN_DELETE | IN_ACCESS;
        inotify_add_watch(fd, current->layer_loc.c_str(), watch_flags);
        while(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
            i = 0;
            length = static_cast<int>(read(fd, buffer, BUF_LEN ));

            if ( length < 0 ) {
                perror( "read" );
            }

            while ( i < length ) {
                struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
                if ( event->len ) {
                    if ( event->mask & IN_CREATE) {
                        if (event->mask & IN_ISDIR)
                            printf( "The directory %s was Created.\n", event->name );
                        else
                            printf( "The file %s was Created with WD %d\n", event->name, event->wd );
                    }

                    if ( event->mask & IN_MODIFY) {
                        if (event->mask & IN_ISDIR)
                            printf( "The directory %s was modified.\n", event->name );
                        else
                            printf( "The file %s was modified with WD %d\n", event->name, event->wd );
                    }

                    if ( event->mask & IN_DELETE) {
                        if (event->mask & IN_ISDIR)
                            printf( "The directory %s was deleted.\n", event->name );
                        else
                            printf( "The file %s was deleted with WD %d\n", event->name, event->wd );
                    }


                    i += EVENT_SIZE + event->len;
                }
            }
        }
        /* Clean up*/
        inotify_rm_watch( fd, wd );
        close(fd);
        return ServerStatus::SERVER_SUCCESS;
    }
public:
    HardwareMonitor():num_monitors(0),event_queue("HARDWARE_QUEUE",CONF->is_server,CONF->my_server,CONF->num_servers){
        Layer* current=Layer::FIRST;
        while(current != nullptr){
            if(current->io_client_type == IOClientType::POSIX_FILE){
                num_monitors++;
            }
            current=current->next;
        }
    }
    ServerStatus AsyncMonitor(){
        monitor_threads = new std::thread[num_monitors];
        monitor__exit_signal = new std::promise<void>[num_monitors];
        Layer* current=Layer::FIRST;
        int i = 0;
        while(current != nullptr){
            if(current->io_client_type == IOClientType::POSIX_FILE){
                std::future<void> futureObj = monitor__exit_signal[i].get_future();
                monitor_threads[i]=std::thread (&HardwareMonitor::AsyncMonitorInternal, this, std::move(futureObj),current);
            }
            current=current->next;
        }

        return ServerStatus::SERVER_SUCCESS;
    }
    std::vector<Event> fetchHardwareEvents(){

    }
    void stop(){
        for (int i = 0; i < num_monitors; ++i) {
            /* Issue server kill signals */
            monitor__exit_signal[i].set_value();
            /* wait for servers to end */
            monitor_threads[i].join();
        }
    }
};


#endif //HFETCH_HARDWAREMONITOR_H

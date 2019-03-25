//
// Created by hariharan on 3/19/19.
//

#include "hardware_monitor.h"

std::vector<Event> HardwareMonitor::FetchEvents() {
    AutoTrace trace = AutoTrace("HardwareMonitor::FetchEvents");
    event_queue.WaitForElement(CONF->my_server);
    std::vector<Event> events=std::vector<Event>();
    auto result = event_queue.Pop(CONF->my_server);
    if(result.first){
        events.push_back(result.second);
    }
    return events;
}

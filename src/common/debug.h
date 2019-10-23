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
/*-------------------------------------------------------------------------
*
* Created: debug.h
* June 5 2018
* Hariharan Devarajan <hdevarajan@hdfgroup.org>
*
* Purpose: Defines debug macros for Hermes.
*
*-------------------------------------------------------------------------
*/

#ifndef HERMES_DEBUG_H
#define HERMES_DEBUG_H

#include <iostream>
#include <csignal>
#include <unistd.h>
#include <execinfo.h>
#include <tuple>
#include "data_structure.h"
#include <thread>


/**
 * Handles signals and prints stack trace.
 *
 * @param sig
 */

inline void handler(int sig) {
    void *array[10];
    size_t size;
    // get void*'s for all entries on the stack
    size = backtrace(array, 300);
    int rank, comm_size;
    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(0);
}
inline void addSignals(){
    signal(SIGABRT, handler);
    signal(SIGSEGV, handler);
    signal(SIGFPE, handler);
    signal(SIGPIPE, handler);
    signal(SIGBUS, handler);
}
/**
 * various macros to print variables and messages.
 */

#ifdef HERMES_DEBUG
#define DBGVAR(var) \
std::cout << "DBG: " << __FILE__ << "(" << __LINE__ << ") "\
       << #var << " = [" << (var) << "]" << std::endl

#define DBGVAR2(var1, var2) \
  std::cout << "DBG: " << __FILE__ << "(" << __LINE__ << ") "\
       << #var1 << " = [" << (var1) << "]"\
       << #var2 << " = [" << (var2) << "]"  << std::endl
#define DBGVAR3(var1, var2, var3) \
  std::cout << "DBG: " << __FILE__ << "(" << __LINE__ << ") "\
       << #var1 << " = [" << (var1) << "]"\
       << #var2 << " = [" << (var2) << "]"\
       << #var3 << " = [" << (var3) << "]"  << std::endl

#define DBGMSG(msg) \
  std::cout << "DBG: " << __FILE__ << "(" << __LINE__ << ") " \
       << msg << std::endl
#else
#define DBGVAR(var)
#define DBGVAR2(var1, var2)
#define DBGVAR3(var1, var2, var3)
#define DBGMSG(msg)
#endif

/**
 * Time all functions and instrument it
 */

#include <stack>
#include <string>
#include <iostream>
#include <stdarg.h>
#include <chrono>
#include <mpi.h>
#include <string>
#include <sstream>

class Timer {
private:
    std::chrono::high_resolution_clock::time_point t1;
    double time_elapsed;
public:
    Timer():time_elapsed(0){}
    void startTime() {
        t1 = std::chrono::high_resolution_clock::now();
        time_elapsed=0;
    }
    void pauseTime() {
        auto t2 = std::chrono::high_resolution_clock::now();
        time_elapsed +=  std::chrono::duration_cast<std::chrono::nanoseconds>(
                t2 - t1).count()/1000000.0;
    }
    void resumeTime() {
        t1 = std::chrono::high_resolution_clock::now();
    }
    double endTime(){
        auto t2 = std::chrono::high_resolution_clock::now();
        time_elapsed +=  std::chrono::duration_cast<std::chrono::nanoseconds>(
                t2 - t1).count()/1000000.0;
        return time_elapsed;
    }

    double getTimeElapsed(){
        return time_elapsed;
    }
};



/**
 * Implement Auto tracing Mechanism.
 */
using namespace std;
class AutoTrace
{
    Timer timer;
    static int rank,item;

public:
    template <typename... Args>
    AutoTrace(std::string string,Args... args):m_line(string)
    {
        char thread_name[256];
        pthread_getname_np(pthread_self(), thread_name,256);
        std::stringstream stream;
        stream << "\033[31m";
        if(rank == -1) MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#if defined(HERMES_TRACE) || defined(HERMES_TIMER)
        stream <<++item<<";"<<thread_name<<";"<< rank << ";" <<m_line << ";";
#endif
#if  defined(HERMES_TIMER)
        stream <<";;";
#endif
#ifdef HERMES_TRACE
        auto args_obj = std::make_tuple(args...);
        const ulong args_size = std::tuple_size<decltype(args_obj)>::value;
        stream << "args(";

        if(args_size == 0) stream << "Void";
        else{
            static_for<args_size>( [&](auto w){
                    stream << std::get<w.n>(args_obj) << ", ";
            });
        }
        stream << ");";
#endif
#if defined(HERMES_TRACE) || defined(HERMES_TIMER)
        stream <<"start"<< endl;
        stream << "\033[00m";
        cout << stream.str();
#endif
#ifdef HERMES_TIMER
        timer.startTime();
#endif
    }

    ~AutoTrace()
    {
        std::stringstream stream;
        char thread_name[256];
        pthread_getname_np(pthread_self(), thread_name,256);
        stream << "\033[31m";
#if defined(HERMES_TRACE) || defined(HERMES_TIMER)
        stream <<item-- <<";"<<std::string(thread_name)<<";"<< rank << ";" << m_line << ";";
#endif
#if defined(HERMES_TRACE)
        stream  <<";";
#endif
#ifdef HERMES_TIMER
        double end_time=timer.endTime();
        stream  <<end_time<<";msecs;";
#endif
#if defined(HERMES_TRACE) || defined(HERMES_TIMER)
        stream  <<"finish"<< endl;
        stream << "\033[00m";
        cout << stream.str();
#endif
    }
private:
    string m_line;
};



#endif //HERMES_DEBUG_H

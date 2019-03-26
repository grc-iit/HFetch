/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5. The full HDF5 copyright notice, including      *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html. COPYING can be found at the root    *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page. It can also be found at      *
 * http://hdfgroup.org/HDF5/doc/Copyright.html. If you do not have           *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
public:
    void startTime() {
        t1 = std::chrono::high_resolution_clock::now();
    }
    double endTime(){
        auto t2 = std::chrono::high_resolution_clock::now();
        auto t =  std::chrono::duration_cast<std::chrono::nanoseconds>(
                t2 - t1).count()/1000000.0;
        return t;
    }
private:
    std::chrono::high_resolution_clock::time_point t1;
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

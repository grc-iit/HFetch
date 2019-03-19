//
// Created by HariharanDevarajan on 2/1/2019.
//

#ifndef HERMES_PROJECT_DISTRIBUTEDPRIORITYQUEUE_H
#define HERMES_PROJECT_DISTRIBUTEDPRIORITYQUEUE_H

/**
 * Include Headers
 */
/** Standard C++ Headers**/
#include <iostream>
#include <functional>
#include <utility>
/** MPI Headers**/
#include <mpi.h>
/** RPC Lib Headers**/
#include "../../../../external/rpclib/include/rpc/server.h"
#include "../../../../external/rpclib/include/rpc/client.h"
/** Boost Headers **/
#include <boost/interprocess/managed_shared_memory.hpp>
#include <queue>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/algorithm/string.hpp>
#include <src/common/rpc_lib.h>
#include <src/common/singleton.h>

/** Namespaces Uses **/
namespace bip=boost::interprocess;

/** Global Typedefs **/
typedef unsigned long long int really_long;

/**
 * This is a Distributed HashMap Class. It uses shared memory + RPC + MPI to achieve the data structure.
 *
 * @tparam MappedType, the value of the HashMap
 */
template<typename MappedType,typename Compare=std::less<MappedType>>
class DistributedPriorityQueue {
private:
    /** Class Typedefs for ease of use **/
    typedef bip::allocator<MappedType, bip::managed_shared_memory::segment_manager> ShmemAllocator;
    typedef std::priority_queue<MappedType, std::vector<MappedType,ShmemAllocator>,Compare> Queue;

    /** Class attributes**/
    int ranks_per_server, comm_size, my_rank,bucket_count,num_servers;
    uint16_t  server_index;
    std::shared_ptr<RPC> rpc;
    really_long memeory_allocated;
    bool is_server;
    boost::interprocess::managed_shared_memory segment;
    std::string name,func_prefix;
    Queue *queue;
    boost::interprocess::interprocess_mutex* mutex;
public:
    /* Constructor to deallocate the shared memory*/
    ~DistributedPriorityQueue(){
        if(is_server) bip::shared_memory_object::remove(name.c_str());
    }

    explicit DistributedPriorityQueue(std::string name_,
                                     int ranks_per_server_ = 1,
                                     really_long memory = 1024ULL * 1024ULL * 1024ULL)
    :is_server(false), ranks_per_server(ranks_per_server_), server_index(0), num_servers(1),
    comm_size(1), my_rank(0), memeory_allocated(memory), name(name_), segment(), queue(),func_prefix(name_){
        /* Initialize MPI rank and size of world */
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        /* get which server the rank will connect to*/
        server_index = static_cast<uint16_t>(my_rank / ranks_per_server);
        /* get the port of that server */
        /* create per server name for shared memory. Needed if multiple servers are spawned on one node*/
        this->name += "_" + std::to_string(server_index);
        /* set whether current rank is server */
        if ((my_rank + 1) % ranks_per_server == 0) is_server = true;
        /* Calculate Max num of servers */
        num_servers = comm_size / ranks_per_server;
        /* if current rank is a server */
        rpc=Singleton<RPC>::GetInstance();
        if (is_server) {
            /* Delete existing instance of shared memory space*/
            bip::shared_memory_object::remove(name.c_str());
            /* allocate new shared memory space */
            segment=bip::managed_shared_memory(bip::create_only,name.c_str(),memeory_allocated);
            ShmemAllocator alloc_inst (segment.get_segment_manager());
            /* Construct Hashmap in the shared memory space. */
            queue = segment.construct<Queue>("Queue")(Compare(),alloc_inst);
            mutex = segment.construct<bip::interprocess_mutex>("mtx")();
            /* Create a RPC server and map the methods to it. */
            rpc=Singleton<RPC>::GetInstance();
            std::function<bool(MappedType,uint16_t)> pushFunc(std::bind(&DistributedPriorityQueue<MappedType,Compare>::Push, this, std::placeholders::_1, std::placeholders::_2));
            std::function<std::pair<bool,MappedType>(uint16_t)> popFunc(std::bind(&DistributedPriorityQueue<MappedType,Compare>::Pop, this, std::placeholders::_1));
            std::function<std::pair<bool,MappedType>(uint16_t)> topFunc(std::bind(&DistributedPriorityQueue<MappedType,Compare>::Top, this, std::placeholders::_1));
            std::function<size_t(uint16_t)> sizeFunc(std::bind(&DistributedPriorityQueue<MappedType,Compare>::Size, this, std::placeholders::_1));
            rpc->bind(func_prefix+"_Push", pushFunc);
            rpc->bind(func_prefix+"_Pop", popFunc);
            rpc->bind(func_prefix+"_Top", topFunc);
            rpc->bind(func_prefix+"_Size", sizeFunc);
        }
        /* Make clients wait untill all servers reach here*/
        MPI_Barrier(MPI_COMM_WORLD);
        /* Map the clients to their respective memory pools */
        if(!is_server){
            segment=bip::managed_shared_memory(bip::open_only,name.c_str());
            std::pair<Queue*, bip::managed_shared_memory::size_type> res;
            res = segment.find<Queue> ("Queue");
            queue = res.first;
            std::pair<bip::interprocess_mutex *, bip::managed_shared_memory::size_type> res2;
            res2 = segment.find<bip::interprocess_mutex>("mtx");
            mutex = res2.first;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    /**
     * Push the data into the queue. Uses key to decide the server to hash it to,
     * @param key, the key for put
     * @param data, the value for put
     * @return bool, true if Put was successful else false.
     */
    bool Push(MappedType data, uint16_t key_int){
        if(key_int == server_index){
            bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
            queue->push(data);
            return true;
        }else{
            return rpc->call(key_int,func_prefix+"_Push", data).template as<bool>();
        }
    }
    /**
     * Get the data from the queue. Uses key_int to decide the server to hash it to,
     * @param key_int, key_int to know which server
     * @return return a pair of bool and Value. If bool is true then data was found
     *          and is present in value part else bool is set to false
     */
    std::pair<bool,MappedType> Pop(uint16_t key_int) {
        if (key_int == server_index) {
            bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
            if(queue->size()>0){
                MappedType value= queue->top();
                queue->pop();
                return std::pair<bool,MappedType>(true,value);
            }
            return std::pair<bool,MappedType>(false,MappedType());;
        } else {
            return rpc->call(key_int,func_prefix+"_Pop").template as<std::pair<bool, MappedType>>();
        }
    }

    /**
     * Get the data from the queue. Uses key_int to decide the server to hash it to,
     * @param key_int, key_int to know which server
     * @return return a pair of bool and Value. If bool is true then data was found
     *          and is present in value part else bool is set to false
     */
    std::pair<bool,MappedType> Top(uint16_t key_int) {
        if (key_int == server_index) {
            bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
            if(queue->size()>0){
                MappedType value= queue->top();
                return std::pair<bool,MappedType>(true,value);
            }
            return std::pair<bool,MappedType>(false,MappedType());;
        } else {
            return rpc->call(key_int,func_prefix+"_Pop").template as<std::pair<bool, MappedType>>();
        }
    }

    /**
     * Get the size of the queue. Uses key_int to decide the server to hash it to,
     * @param key_int, key_int to know which server
     * @return return a size of the queue
     */
    size_t Size(uint16_t key_int) {
        if (key_int == server_index) {
            //bip::scoped_lock<bip::interprocess_mutex> lock(*mutex);
            size_t value= queue->size();
            return value;
        } else {
            return rpc->call(key_int,func_prefix+"_Size").template as<size_t>();;
        }
    }

};

#endif //HERMES_PROJECT_DISTRIBUTEDPRIORITYQUEUE_H

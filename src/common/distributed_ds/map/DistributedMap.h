//
// Created by HariharanDevarajan on 2/1/2019.
//

#ifndef HERMES_PROJECT_DISTRIBUTEDMAP_H
#define HERMES_PROJECT_DISTRIBUTEDMAP_H

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
#include <rpc/server.h>
#include <rpc/client.h>
/** Boost Headers **/
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/algorithm/string.hpp>
#include <src/common/rpc_lib.h>
#include <src/common/singleton.h>
/** Global Typedefs **/
typedef unsigned long long int really_long;

/**
 * This is a Distributed HashMap Class. It uses shared memory + RPC + MPI to achieve the data structure.
 *
 * @tparam MappedType, the value of the HashMap
 */




template<typename KeyType, typename MappedType, typename Compare = std::less<KeyType>>
class DistributedMap {
private:
    std::hash<KeyType> keyHash;
    /** Class Typedefs for ease of use **/
    typedef std::pair<const KeyType, MappedType> ValueType;
    typedef boost::interprocess::allocator<ValueType, boost::interprocess::managed_shared_memory::segment_manager> ShmemAllocator;
    typedef boost::interprocess::map<KeyType, MappedType, Compare, ShmemAllocator> MyMap;
    /** Class attributes**/
    int ranks_per_server, comm_size, my_rank,bucket_count,num_servers;
    uint16_t  server_index;
    std::shared_ptr<RPC> rpc;
    really_long memeory_allocated;
    bool is_server;
    boost::interprocess::managed_shared_memory segment;
    std::string name,func_prefix;
    MyMap *mymap;
    boost::interprocess::interprocess_mutex* mutex;

public:

    /* Constructor to deallocate the shared memory*/
    ~DistributedMap(){
        boost::interprocess::shared_memory_object::remove(name.c_str());
    }

    explicit DistributedMap(){}
    explicit DistributedMap(std::string name_,
                            int ranks_per_server_ = 1,
                            really_long memory = 1024ULL * 1024ULL * 1024ULL)
    :is_server(false), ranks_per_server(ranks_per_server_), server_index(0), num_servers(1),
    comm_size(1), my_rank(0), memeory_allocated(memory), name(name_), segment(), mymap(),func_prefix(name_){
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
            boost::interprocess::shared_memory_object::remove(name.c_str());
            /* allocate new shared memory space */
            segment=boost::interprocess::managed_shared_memory(boost::interprocess::create_only,name.c_str(),memeory_allocated);
            ShmemAllocator alloc_inst (segment.get_segment_manager());
            /* Construct Hashmap in the shared memory space. */
            mymap = segment.construct<MyMap>(name.c_str())(Compare() ,alloc_inst);
            mutex = segment.construct<boost::interprocess::interprocess_mutex>("mtx")();
            /* Create a RPC server and map the methods to it. */
            std::function<bool(KeyType,MappedType)> putFunc(std::bind(&DistributedMap<KeyType,MappedType,Compare>::Put, this, std::placeholders::_1 , std::placeholders::_2));
            std::function<std::pair<bool,MappedType>(KeyType)> getFunc(std::bind(&DistributedMap<KeyType,MappedType,Compare>::Get, this, std::placeholders::_1 ));
            std::function<std::vector<std::pair<KeyType,MappedType>>(KeyType)> containsInServerFunc(std::bind(&DistributedMap<KeyType,MappedType,Compare>::ContainsInServer, this, std::placeholders::_1 ));
            std::function<std::pair<bool,MappedType>(KeyType)> eraseFunc(std::bind(&DistributedMap<KeyType,MappedType,Compare>::Erase, this, std::placeholders::_1 ));
            std::function<std::vector<std::pair<KeyType,MappedType>>(void)> getAllDataInServerFunc(std::bind(&DistributedMap<KeyType,MappedType,Compare>::GetAllDataInServer, this ));
            rpc->bind(func_prefix+"_Put", putFunc);
            rpc->bind(func_prefix+"_Get", getFunc);
            rpc->bind(func_prefix+"_Erase", eraseFunc);
            rpc->bind(func_prefix+"_GetAllData", getAllDataInServerFunc);
            rpc->bind(func_prefix+"_Contains", containsInServerFunc);
        }
        /* Make clients wait untill all servers reach here*/
        MPI_Barrier(MPI_COMM_WORLD);
        /* Map the clients to their respective memory pools */
        if(!is_server){
            segment=boost::interprocess::managed_shared_memory(boost::interprocess::open_only,name.c_str());
            std::pair<MyMap*,boost::interprocess:: managed_shared_memory::size_type> res;
            res = segment.find<MyMap> (name.c_str());
            mymap = res.first;
            std::pair<boost::interprocess::interprocess_mutex *, boost::interprocess::managed_shared_memory::size_type> res2;
            res2 = segment.find<boost::interprocess::interprocess_mutex>("mtx");
            mutex = res2.first;
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    /**
     * Put the data into the hashmap. Uses key to decide the server to hash it to,
     * @param key, the key for put
     * @param data, the value for put
     * @return bool, true if Put was successful else false.
     */
    bool Put(KeyType key, MappedType data){
        size_t key_hash = keyHash(key);
        uint16_t key_int = static_cast<uint16_t>(key_hash % num_servers);
        if(key_int == server_index){
            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*mutex);
            typename MyMap::iterator iterator = mymap->find(key);
            if (iterator != mymap->end()) {
                mymap->erase(iterator);
            }
            mymap->insert(std::pair<KeyType,MappedType>(key, data));
            return true;
        }else{
            return rpc->call(key_int,func_prefix+"_Put",key, data).template as<bool>();
        }
    }
    /**
     * Get the data into the hashmap. Uses key to decide the server to hash it to,
     * @param key, key to get
     * @return return a pair of bool and Value. If bool is true then data was found
     *          and is present in value part else bool is set to false
     */
    std::pair<bool,MappedType> Get(KeyType key) {
        size_t key_hash = keyHash(key);
        uint16_t key_int = key_hash % num_servers;
        if (key_int == server_index) {
            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*mutex);
            typename MyMap::iterator iterator = mymap->find(key);
            if (iterator != mymap->end()) {
                return std::pair<bool, MappedType>(true, iterator->second);
            } else {
                return std::pair<bool, MappedType>(false, MappedType());
            }
        } else {
            return rpc->call(key_int,func_prefix+"_Get",key).template as<std::pair<bool, MappedType>>();
        }
    }

    std::pair<bool,MappedType> Erase(KeyType key) {
        size_t key_hash = keyHash(key);
        uint16_t key_int = key_hash % num_servers;
        if (key_int == server_index) {
            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*mutex);
            size_t s = mymap->erase(key);
            return std::pair<bool, MappedType>(s>0, MappedType());
        } else {
            return rpc->call(key_int,func_prefix+"_Erase",key).template as<std::pair<bool, MappedType>>();
        }
    }

    /**
     * Get the data into the hashmap. Uses key to decide the server to hash it to,
     * @param key, key to get
     * @return return a pair of bool and Value. If bool is true then data was found
     *          and is present in value part else bool is set to false
     */
    std::vector<std::pair<KeyType,MappedType>> Contains(KeyType key) {
        std::vector<std::pair<KeyType,MappedType>> final_values=std::vector<std::pair<KeyType,MappedType>>();
        auto current_server=ContainsInServer(key);
        final_values.insert(final_values.end(),current_server.begin(),current_server.end());
        int total_servers=comm_size/ranks_per_server;
        for(int i=0;i<total_servers;++i){
            if(i!=server_index){
                auto server=rpc->call(i,func_prefix+"_Contains",key).template as<std::vector<std::pair<KeyType,MappedType>>>();
                final_values.insert(final_values.end(),server.begin(),server.end());
            }
        }
        return final_values;

    }

    std::vector<std::pair<KeyType,MappedType>> GetAllData() {
        std::vector<std::pair<KeyType,MappedType>> final_values=std::vector<std::pair<KeyType,MappedType>>();
        auto current_server=GetAllDataInServer();
        final_values.insert(final_values.end(),current_server.begin(),current_server.end());
        int total_servers=comm_size/ranks_per_server;
        for(int i=0;i<total_servers;++i){
            if(i!=server_index){
                auto server=rpc->call(i,func_prefix+"_GetAllData").template as<std::vector<std::pair<KeyType,MappedType>>>();
                final_values.insert(final_values.end(),server.begin(),server.end());
            }
        }
        return final_values;

    }

    std::vector<std::pair<KeyType,MappedType>> ContainsInServer(KeyType key) {
        std::vector<std::pair<KeyType,MappedType>> final_values = std::vector<std::pair<KeyType,MappedType>>();
        {
            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*mutex);
            typename MyMap::iterator lower_bound;
            size_t size=mymap->size();
            if (size == 0){

            }
            else if (size == 1){
                lower_bound = mymap->begin();
                final_values.insert(final_values.end(),std::pair<KeyType,MappedType>(lower_bound->first, lower_bound->second));
            }
            else {
                lower_bound = mymap->lower_bound(key);
                if(lower_bound==mymap->end()) return final_values;
                if (lower_bound != mymap->begin()) {
                    --lower_bound;
                    if (!key.Contains(lower_bound->first)) lower_bound++;
                }
                while (lower_bound != mymap->end()) {
                    if(!(key.Contains(lower_bound->first) || lower_bound->first.Contains(key))) break;
                    final_values.insert(final_values.end(),std::pair<KeyType,MappedType>(lower_bound->first, lower_bound->second));
                    lower_bound++;


                }
            }

        }
        return final_values;
    }
    std::vector<std::pair<KeyType,MappedType>> GetAllDataInServer() {
        std::vector<std::pair<KeyType,MappedType>> final_values = std::vector<std::pair<KeyType,MappedType>>();
        {
            boost::interprocess::scoped_lock<boost::interprocess::interprocess_mutex> lock(*mutex);
            typename MyMap::iterator lower_bound;
            lower_bound = mymap->begin();
            while (lower_bound != mymap->end()){
                final_values.insert(final_values.end(),std::pair<KeyType,MappedType>(lower_bound->first, lower_bound->second));
                lower_bound++;
            }
        }
        return final_values;
    }
};
#endif //HERMES_PROJECT_DISTRIBUTEDMAP_H

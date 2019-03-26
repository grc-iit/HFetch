cmake_minimum_required(VERSION 3.10)
project(hfetch VERSION 1.0.0 DESCRIPTION "Heirarchical Prefetching Platform")

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(external/rpclib)
include_directories(external/rpclib/include)
include_directories(external/rpclib/dependencies/include)

set(INSTALL_DIR $ENV{INSTALL_DIR})
message(INFO " INSTALL_DIR ${INSTALL_DIR}")

set(LIB_FLAGS -L${CMAKE_BINARY_DIR}/external/rpclib/ -lrpc  -L${INSTALL_DIR}/lib/ -lmpi -lpthread -lrt -ldl -lboost_filesystem )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${INSTALL_DIR}/include/  -L${CMAKE_BINARY_DIR}/ -w ")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHERMES_DEBUG")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHERMES_TIMER")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHERMES_TRACE")


include_directories(${INSTALL_DIR}/include/)
include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories(${PROJECT_SOURCE_DIR})
#HFetch Common
set(HFETCH_COMMON_INT_SRC   src/common/singleton.h
                        src/common/distributed_ds/clock/global_clock.h src/common/distributed_ds/sequencer/global_sequence.h
                        src/common/distributed_ds/communication/rpc_lib.h
                        src/common/distributed_ds/map/DistributedMap.h
                        src/common/distributed_ds/multimap/DistributedMultiMap.h
                        src/common/distributed_ds/hashmap/DistributedHashMap.h
                        src/common/distributed_ds/priority_queue/DistributedPriorityQueue.h
                        src/common/distributed_ds/queue/DistributedMessageQueue.h
                        src/common/constants.h
                        src/common/debug.h
                        src/common/data_structure.h
                        src/common/configuration_manager.h
                        src/common/macros.h src/common/typedefs.h
                        src/common/enumerations.h src/common/data_structures.cpp
                        src/common/io_clients/io_client_factory.h
                        src/common/io_clients/file_client.cpp
                        src/common/io_clients/file_client.h
                        src/common/io_clients/memory_client.cpp
                        src/common/io_clients/memory_client.h
                        src/common/io_clients/io_client.h
                        src/common/io_clients/data_manager.h
                        src/common/util.h
        src/common/debug.cpp)
#Hfetch Server
set(HFETCH_SERVER_INT_SRC
        src/server/server.h
        src/server/file_segment_auditor.cpp src/server/file_segment_auditor.h
        src/server/event_manager.cpp src/server/event_manager.h
        src/server/hardware_monitor.cpp src/server/hardware_monitor.h
        src/server/dpe/dpe_factory.h src/server/dpe/dpe.h
        src/server/dpe/max_bw_dpe.cpp src/server/dpe/max_bw_dpe.h
        )
#HFetch Library
set(HFETCH_LIB_H_SRC include/hfetch.h)
set(HFETCH_LIB_INT_SRC  ${HFETCH_LIB_H_SRC}
                        src/lib/hfetch.cpp
                        src/lib/metadata_manager.cpp src/lib/metadata_manager.h)
#Complete hfetch source
set(HFETCH_SRC ${HFETCH_COMMON_INT_SRC} ${HFETCH_LIB_INT_SRC} ${HFETCH_SERVER_INT_SRC})

#lib source
set(HFETCH_LIB_SRC  ${HFETCH_SRC})

add_library(hfetch SHARED ${HFETCH_LIB_SRC})
add_dependencies(hfetch rpc)
target_link_libraries(hfetch ${LIB_FLAGS})
set_target_properties(hfetch PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(hfetch PROPERTIES SOVERSION 1)
set_target_properties(hfetch PROPERTIES PUBLIC_HEADER ${HFETCH_LIB_H_SRC})
include(GNUInstallDirs)
target_include_directories(hfetch PRIVATE .)
install(TARGETS hfetch LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#HFetch Server
set(HFETCH_SERVER_SRC   ${HFETCH_SRC}
        src/server/main.cpp
)

add_executable(hfetch_server ${HFETCH_SERVER_SRC})
set_target_properties(hfetch_server PROPERTIES ENV TEST_ENV="LD_PRELOAD=-L${CMAKE_BINARY_DIR}/external/rpclib/librpc.so")
add_dependencies(hfetch_server rpc)
target_link_libraries(hfetch_server ${LIB_FLAGS})

#HFetch Test Cases

add_subdirectory(test)
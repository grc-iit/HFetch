cmake_minimum_required(VERSION 3.10)
project(hfetch VERSION 1.0.0 DESCRIPTION "Heirarchical Prefetching Platform")

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(external/rpclib)
include_directories(external/rpclib/include)
include_directories(external/rpclib/dependencies/include)

set(INSTALL_DIR $ENV{INSTALL_DIR})
message(INFO " INSTALL_DIR ${INSTALL_DIR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${INSTALL_DIR}/include/ -L${INSTALL_DIR}/lib/ -L${CMAKE_BINARY_DIR}/")
include_directories(${INSTALL_DIR}/include/)
include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories(${PROJECT_SOURCE_DIR})
#HFetch Common
set(HFETCH_COMMON_SRC src/common/singleton.h src/common/rpc_lib.h src/common/rpc_lib.cpp src/common/distributed_ds/map/DistributedMap.h src/common/distributed_ds/hashmap/DistributedHashMap.h src/common/distributed_ds/priority_queue/DistributedPriorityQueue.h src/common/distributed_ds/queue/DistributedMessageQueue.h)

#HFetch Library
set(HFETCH_LIB_H_SRC include/hfetch.h)
set(HFETCH_LIB_SRC ${HFETCH_COMMON_SRC} ${HFETCH_LIB_H_SRC} src/lib/hfetch.cpp)

add_library(hfetch SHARED ${HFETCH_LIB_SRC})
target_link_libraries(hfetch -lmpi)
set_target_properties(hfetch PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(hfetch PROPERTIES SOVERSION 1)
set_target_properties(hfetch PROPERTIES PUBLIC_HEADER ${HFETCH_LIB_H_SRC})
include(GNUInstallDirs)
target_include_directories(hfetch PRIVATE .)
install(TARGETS hfetch LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

#HFetch Server
set(HFETCH_SERVER_SRC ${HFETCH_COMMON_SRC} src/server/server.cpp)
add_executable(hfetch_server ${HFETCH_SERVER_SRC})
target_link_libraries(hfetch_server -lmpi)

#HFetch Test Cases

add_subdirectory(test)
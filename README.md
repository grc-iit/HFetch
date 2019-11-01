# Introduction

In the era of data-intensive computing, accessing data with a high-throughput and low-latency is more imperative than ever. Data prefetching is a well-known technique for hiding read latency. However, existing solutions do not consider the new deep memory and storage hierarchy and also suffer from under-utilization of prefetching resources and unnecessary evictions. Additionally, existing approaches implement a client-pull model where understanding the application's I/O behavior drives prefetching decisions. Moving towards exascale, where machines run multiple applications concurrently by accessing files in a workflow, a more data-centric approach can resolve challenges such as cache pollution and redundancy. In this study, we present HFetch, a truly hierarchical data prefetcher that adopts a server-push approach to data prefetching. We demonstrate the benefits of such an approach. Results show 10-35% performance gains over existing prefetchers and over 50% when compared to systems with no prefetching.

# Who uses

HFetch is used within [Hermes](http://www.cs.iit.edu/~scs/assets/projects/Hermes/Hermes.html) to perform hierarchical prefetching of data based on various application and system characteristics.

# How it works
-   HFetch contains a library (libhfetch) which transparently intercepts all POSIX and HDF5 calls.
-   HFetch has a server that should be deployed on each node and performs data-centric prefetching.
    

# Dependencies
-   Cmake > 3.13,3
-   Gcc > 7.3    
-   MPICH > 3.3
-   HDF5 > 10
-   H5Part
-   Boost > 1.69.0
-   RapidJSON
-   RPClib
    
# Installation

## Cmake

`mkdir build && cd build`
`cmake ../`
`make -j8 && make install`

# Usage

## Library

Applications need to link their executable with libhfetch. Internally, HFetch lib intercepts file operations (fopen,fclose, etc.) defined using POSIX and HDF5 calls of the applications.

## Server

Each node of HFetch should deploy a server instance.

`
./hfetch_server
`

# Running Tests

## Ctest

### VPIC:

`ctest -R vpic`

### WRF:

`ctest -R wrf`

### All:

`ctest`

# License:

# Acknowledgments:
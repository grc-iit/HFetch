# README #

This README would normally document whatever steps are necessary to get your application up and running.

### What is this repository for? ###

* Quick summary
* Version
* [Learn Markdown](https://bitbucket.org/tutorials/markdowndemo)

### How do I get set up? ###

* Summary of set up
* Configuration
* Dependencies
#mpich 3.3+
`./configure --prefix=/home/hdevarajan/software/install --enable-fast=03 --enable-shared --enable-romio --enable-threads --disable-fortran --disable-fc --enable-onsig --enable-debuginfo --enable-g=handle`

* Database configuration
* How to run tests
#servers

#clients
`mpirun -n $((64*8)) -f hostfile ./basic -s 8 -r 64`
* Deployment instructions

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Who do I talk to? ###

* Repo owner or admin
* Other community or team contact
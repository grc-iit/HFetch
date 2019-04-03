#!/bin/bash
SERVER_NODES=$(cat pfs_servers)
SCRIPT_DIR=/home/hdevarajan/projects/hfetch/scripts/environment
for server_node in $SERVER_NODES
do
ssh $server_node /bin/bash << EOF
rm -rf /mnt/hdd/hdevarajan/storage/*
killall pvfs2-server
mkdir -p /mnt/hdd/hdevarajan/storage
pvfs2-server -f -a ${server_node} ${SCRIPT_DIR}/pfs.conf
pvfs2-server -a ${server_node} ${SCRIPT_DIR}/pfs.conf
ps -aef | grep pvfs2
EOF
done

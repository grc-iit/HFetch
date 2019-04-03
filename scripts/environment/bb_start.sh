#!/bin/bash
SERVER_NODES=$(cat bb_servers)
SCRIPT_DIR=/home/hdevarajan/projects/hfetch/scripts/environment
for server_node in $SERVER_NODES
do
ssh $server_node /bin/bash << EOF
rm -rf /mnt/ssd/hdevarajan/storage/*
killall pvfs2-server
mkdir -p /mnt/sdd/hdevarajan/storage
pvfs2-server -f -a ${server_node} ${SCRIPT_DIR}/bb.conf
pvfs2-server -a ${server_node} ${SCRIPT_DIR}/bb.conf
ps -aef | grep pvfs2
EOF
done

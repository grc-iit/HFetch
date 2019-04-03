#!/bin/bash
SERVER_NODES=$(cat pfs_servers)
SCRIPT_DIR=/home/hdevarajan/projects/hfetch/scripts/environment
for server_node in $SERVER_NODES
do
ssh $server_node /bin/bash << EOF
rm -rf /mnt/hdd/hdevarajan/storage/*
ps -aef | grep pvfs | awk '{print $2}' | xargs kill -9
killall pvfs2-server
ps -aef | grep pvfs2
EOF
done

#!/bin/bash
CLIENT_NODES=$(cat clients)
SCRIPT_DIR=/home/hdevarajan/projects/hfetch/scripts/environment
for client_node in $CLIENT_NODES
do
echo "Starting client on $client_node"
ssh $client_node /bin/bash << EOF
sudo umount --force /mnt/nvme/hdevarajan/pfs
sudo umount --force /mnt/nvme/hdevarajan/bb
sudo kill-pvfs2-client
sudo rmmod pvfs2
mount | grep pvfs2
EOF
echo "Started client on $client_node"
done

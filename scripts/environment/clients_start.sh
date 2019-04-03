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
mkdir -p /mnt/nvme/hdevarajan/pfs /mnt/nvme/hdevarajan/bb
sudo insmod ${ORANGEFS_KO}/pvfs2.ko
sudo ${ORANGEFS_PATH}/sbin/pvfs2-client -p ${ORANGEFS_PATH}/sbin/pvfs2-client-core
sudo mount -t pvfs2 tcp://ares-stor-01-40g:3334/orangefs /mnt/nvme/hdevarajan/pfs
sudo mount -t pvfs2 tcp://ares-stor-26-40g:3334/orangefs /mnt/nvme/hdevarajan/bb
mount | grep pvfs2
EOF
echo "Started client on $client_node"
done

#!/bin/bash
# Copyright (C) 2019  SCS Lab <scs-help@cs.iit.edu>, Hariharan
# Devarajan <hdevarajan@hawk.it.edu>, Xian-He Sun <sun@iit.edu>
#
# This file is part of HFetch
# 
# HFetch is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program.  If not, see
# <http://www.gnu.org/licenses/>.
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

#!/bin/bash
# Copyright (C) 2019  SCS Lab <scs-help@cs.iit.edu>, Hariharan
# Devarajan <hdevarajan@hawk.iit.edu>, Xian-He Sun <sun@iit.edu>
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

#!/bin/ash
# 
# Running program
echo "Bello BiscuitOS" > /dev/shm/BiscuitOS.mem
BiscuitOS-PAGING-PF-SHMEM-DEVSHM-default

# SHMEM Information
echo ""
echo "ls -l /dev/shm/BiscuitOS.mem"
ls -l /dev/shm/BiscuitOS.mem
echo ""
echo "free"
free

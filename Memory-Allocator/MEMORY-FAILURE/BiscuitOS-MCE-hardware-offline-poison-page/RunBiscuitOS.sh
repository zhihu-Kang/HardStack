#!/bin/ash

BiscuitOS-MCE-hardware-offline-poison-page-default &
sleep 0.2

# Obtain Poison page address
POISON_PADDR=$(cat /tmp/.posion_paddr.txt)
# Hardware offline poison page
[ ! -f /sys/devices/system/memory/hard_offline_page ] && echo "Pls Enable CONFIG_MEMORY_FAILURE" && exit 1
echo ${POISON_PADDR} > /sys/devices/system/memory/hard_offline_page
echo "Offline Poison Page ${POISON_PADDR}"

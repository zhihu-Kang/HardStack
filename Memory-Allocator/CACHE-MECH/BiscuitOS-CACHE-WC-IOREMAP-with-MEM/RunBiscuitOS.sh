#!/bin/ash
insmod /lib/modules/$(uname -r)/extra/BiscuitOS-CACHE-WC-IOREMAP-with-MEM-default.ko
cat /sys/kernel/debug/x86/pat_memtype_list
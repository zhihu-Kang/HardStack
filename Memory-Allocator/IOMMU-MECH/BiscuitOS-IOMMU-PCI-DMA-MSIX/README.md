BiscuitOS IOMMU Usage
======================================

## Deploy

#### 1. Goto Qemu source code directory

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/
```

**VERSION** is kernel version and **ARCH** is architecture of platformat. qemu-system is soft link from variable version qemu.

#### 2. Create BiscuitOS Device Module directory

Create BiscuitOS directory on **hw/**  directory, then modify Makefile.objs and Kconfig, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/
mkdir BiscuitOS
vi Makefile.objs

# Add context
devices-dirs-$(CONFIG_SOFTMMU) += BiscuitOS/

vi Kconfig

# Add context
source BiscuitOS/Kconfig
```

#### 3. Download module source code

We need download and copy source code from BiscuitOS, sach as:

```
cd BiscuitOS/
make linux-6.0-x86_64_defconfig
make menuconfig 

  [*] Package --->
      [*] IOMMU: Input/Output Memory Management Unit  --->
          [*]   IOMMU: PCI DMA MSIX with IOMMU  --->
  -*- DIY BiscuitOS/Broiler Hardware  --->
      -*-   Intel Q35
      -*-     vIOMMU
      -*-   BiscuitOS PCI/PCIe DMA with MSIX Interrupt

make
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-PCI-DMA-QEMU-DEVICE-MSIX-default/
make download
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/BiscuitOS/
cp BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-PCI-DMA-QEMU-DEVICE-MSIX-default/BiscuitOS-PCI-DMA-QEMU-DEVICE-MSIX-default ./ -rf
```

And the modify Makefile.objs and Kconfig under **hw/BiscuitOS/** directory, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/BiscuitOS/
vi Makefile.objs

# Add context
common-obj-$(CONFIG_BISCUITOS_PCI_DMA_MSIX) += BiscuitOS-PCI-DMA-QEMU-DEVICE-MSIX-default/

vi Kconfig

# Add context
source BiscuitOS-PCI-DMA-QEMU-DEVICE-MSIX-default/Kconfig
```

#### 4. Enable BiscuitOS Macro

We need create some macro to enable BiscuitOS module, **CONFIG_BISCUITOS_PCI_DMA_MSIX** macro is used to enable BiscuitOS PCI module, **CONFIG_BISCUITOS_DMA** macro is used to enable BiscuitOS DMA module. We need enable those on special file, such as on X86 Emulator:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/
vi default-configs/i386-softmmu.mak

# Add context
CONFIG_BISCUITOS_PCI_DMA_MSIX=y

vi config-all-devices.mak

# Add context
CONFIG_BISCUITOS_PCI_DMA_MSIX:=$(findstring y,$(CONFIG_BISCUITOS_PCI_DMA_MSIX)y)
```

#### 5. Add QEMU command

The BiscuitOS PCI module is called from QEMU command, so we need add qemu options for module, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/
vi RunBiscuitOS.sh

# Check and if doesn't exist, then add context
        -hda ${ROOT}/BiscuitOS.img \
+       -device BiscuitOS-PCI-DMA-MSIX \
        -drive file=${ROOT}/Freeze.img,if=virtio \
```

The name of PCI module is **BiscuitOS-PCI-DMA-MSIX**, the options "-device" while invoke on QEMU.

#### 6. Compile QEMU

BiscuitOS provides a auto scripts to compile QEMU, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/
./RunQEMU.sh -b
```

The BiscuitOS is running when compile pass, and the BiscuitOS-pci module is running.

```
 ____  _                _ _    ___  ____  
| __ )(_)___  ___ _   _(_) |_ / _ \/ ___| 
|  _ \| / __|/ __| | | | | __| | | \___ \ 
| |_) | \__ \ (__| |_| | | |_| |_| |___) |
|____/|_|___/\___|\__,_|_|\__|\___/|____/ 
Welcome to BiscuitOS

Please press Enter to activate this console. 
~ # dmesg | grep 1026
[    0.216614] pci 0000:00:04.0: [1026:1991] type 00 class 0x00ff00
~ # 
~ #
```

The ID 1016:1413 is BiscuitOS-PCI-DMA-MSIX device. CONFIG_BISCUITOS_PCI_DMA_MSIX is enable.

## File

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/BiscuitOS/BiscuitOS-PCI-DMA-QEMU-DEVICE-MSIX-default
tree

├── main.c
├── Kconfig
├── Makefile.objs
└── README.md
```

**main.c** is source code for BiscuitOS PCI module, and Kconfig create macro BISCUITOS_PCI_DMA_MSIX, Makefile.objs add it to compiler. the usage of QEMU from README.md.

## Running Device Driver

The end, Running Device Driver on Guest, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-IOMMU-PCI-DMA-MSIX-default
make download
make build


# The BiscuitOS Running:
 ____  _                _ _    ___  ____
| __ )(_)___  ___ _   _(_) |_ / _ \/ ___|
|  _ \| / __|/ __| | | | | __| | | \___ \
| |_) | \__ \ (__| |_| | | |_| |_| |___) |
|____/|_|___/\___|\__,_|_|\__|\___/|____/
Welcome to BiscuitOS

Please press Enter to activate this console. 
/ # insmod /lib/modules/6.0.0/extra/BiscuitOS-IOMMU-PCI-DMA-MSIX-default.ko 
[   10.744538] BiscuitOS_IOMMU_PCI_DMA_MSIX_default: loading out-of-tree module taints kernel.
[   10.750222] PCI Receive Interrupt.
[   10.750342] BUFFER-0: Weclome to BiscuitOS/Broiler  :)��
[   10.750442] BUFFER-1: ̣
[   10.950442] PCI Receive Interrupt.
[   10.950540] BUFFER-0: Weclome to BiscuitOS/Broiler  :)��
[   10.950625] BUFFER-1: Hello BiscuitOS
```

## Blog

```
http://www.biscuitos.cn/blog/BiscuitOS_Catalogue/
```

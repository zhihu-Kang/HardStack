BiscuitOS PCI QEMU Module Usage
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
make linux-5.0-x86_64_defconfig
make menuconfig 

  [*] Package --->
      [*] PCI: Peripheral Component Interconnect --->
          [*] PCI Common Device Driver Module --->
          [*] QEMU emulate PCIe Device (BiscuitOS-pcie) --->

make
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-pci-device-QEMU-emulate-default/
make download
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/BiscuitOS/
cp BiscuitOS/output/linux-${VERSION}-${ARCH}/package/BiscuitOS-pci-device-QEMU-emulate-default/BiscuitOS-pci-device-QEMU-emulate-default ./ -rf
```

And the modify Makefile.objs and Kconfig under **hw/BiscuitOS/** directory, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/BiscuitOS/
vi Makefile.objs

# Add context
common-obj-$(CONFIG_BISCUITOS_PCI) += BiscuitOS-pci-device-QEMU-emulate-default/

vi Kconfig

# Add context
source BiscuitOS-pci-device-QEMU-emulate-default/Kconfig
```

#### 4. Enable BiscuitOS Macro

We need create some macro to enable BiscuitOS module, **CONFIG_BISCUITOS_PCI** macro is used to enable BiscuitOS PCI module, **CONFIG_BISCUITOS_DMA** macro is used to enable BiscuitOS DMA module. We need enable those on special file, such as on X86 Emulator:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/
vi default-configs/i386-softmmu.mak

# Add context
CONFIG_BISCUITOS_PCI=y

vi config-all-devices.mak

# Add context
CONFIG_BISCUITOS_PCI:=$(findstring y,$(CONFIG_BISCUITOS_PCI)y)
```

#### 5. Add QEMU command

The BiscuitOS PCI module is called from QEMU command, so we need add qemu options for module, such as:

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/
vi RunBiscuitOS.sh

# Add context
        -hda ${ROOT}/BiscuitOS.img \
+       -device BiscuitOS-pci \
        -drive file=${ROOT}/Freeze.img,if=virtio \
```

The name of PCI module is **BiscuitOS-pci**, the options "-device" while invoke on QEMU.

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
~ # dmesg | grep 1016
[    0.216614] pci 0000:00:04.0: [1016:1413] type 00 class 0x00ff00
~ # 
~ #
```

The ID 1016:1413 is BiscuitOS-pci device. CONFIG_BISCUITOS_PCI is enable.

## File

```
cd BiscuitOS/output/linux-${VERSION}-${ARCH}/qemu-system/qemu-system/hw/BiscuitOS/BiscuitOS-pci-device-QEMU-emulate-default
tree

├── BiscuitOS-pci.c
├── Kconfig
├── Makefile.objs
└── README.md
```

**BiscuitOS-pci.c** is source code for BiscuitOS PCI module, and Kconfig create macro BISCUITOS_PCI, Makefile.objs add it to compiler. the usage of QEMU from README.md.

## Blog

```
https://biscuitos.github.io/blog/BiscuitOS_Catalogue/
```

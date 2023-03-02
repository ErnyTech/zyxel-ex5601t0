ARCH:=aarch64
SUBTARGET:=ex5601t0
BOARDNAME:=EX5601-T0
CPU_TYPE:=cortex-a53
FEATURES:=squashfs nand ramdisk

KERNELNAME:=Image dtbs

define Target/Description
	Build firmware images for MediaTek MT7986 ARM based boards.
endef

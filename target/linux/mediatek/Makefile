# Copyright (c) 2015 OpenWrt.org
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=mediatek
BOARDNAME:=MediaTek Ralink ARM
#SUBTARGETS:=mt7622 mt7623 mt7629 mt7986 mt7981
# ZYXEL, subtargets for zyxel projects
SUBTARGETS += $(filter mt% dx% ex% wx%,$(shell find * -maxdepth 0 -type d))
FEATURES:=squashfs nand ramdisk fpu

KERNEL_PATCHVER:=5.4
KERNEL_TESTING_PATCHVER:=5.4

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug


$(eval $(call BuildTarget))

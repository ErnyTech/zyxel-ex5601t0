# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2012-2015 OpenWrt.org
# Copyright (C) 2016-2017 LEDE project

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

define Build/sysupgrade-emmc
	rm -f $@.recovery
	mkfs.fat -C $@.recovery 3070

	./gen_$(SUBTARGET)_emmc_img.sh $@ \
		$(IMAGE_KERNEL) \
		$@.recovery \
		$(IMAGE_ROOTFS)
endef

# build squashfs-hashed
define Build/squashfs-hashed
	$(CP) $(KDIR)/root.squashfs $(IMAGE_ROOTFS)
	$(TOPDIR)/scripts/make-squashfs-hashed.sh \
		$(IMAGE_ROOTFS) \
		$(STAGING_DIR_HOST) \
		$(TOPDIR) \
		$(KDIR)/root.squashfs-hashed-summary-$(DEVICE_NAME)
	cat $(KDIR)/root.squashfs-hashed-summary-$(DEVICE_NAME) | \
		$(TOPDIR)/scripts/prepare-dm-verity-uboot-script.sh \
		"$(HASHED_BOOT_DEVICE)" \
		"$(BASIC_KERNEL_CMDLINE)" \
		> $(KDIR)/$(DEVICE_NAME)-u-boot-script
endef

# build fw-ar-ver
get_fw_ar_ver = \
  $(if $(wildcard $(2)),$(shell rm -rf $(2))) \
  $(if $(wildcard $(1)),$(info $(shell $(STAGING_DIR_HOST)/bin/ar-tool fw_ar_table create_ar_conf $(1) $(2)))) \
  $(if $(wildcard $(2)),$(eval include $(2))) \
  $(if $(FW_AR_VER),$(info FW_AR_VER = $(FW_AR_VER)))

define Build/fw-ar-ver
	$(call get_fw_ar_ver,$(ANTI_ROLLBACK_TABLE),$(AUTO_AR_CONF))
endef

# build signed fit
define Build/fit-sign
	$(TOPDIR)/scripts/mkits.sh \
		-D $(DEVICE_NAME) \
		-o $@.its \
		-k $@ \
		$(if $(word 2,$(1)),-d $(word 2,$(1))) -C $(word 1,$(1)) \
		-a $(KERNEL_LOADADDR) \
		-e $(if $(KERNEL_ENTRY),$(KERNEL_ENTRY),$(KERNEL_LOADADDR)) \
		-c $(if $(DEVICE_DTS_CONFIG),$(DEVICE_DTS_CONFIG),"config-1") \
		-A $(LINUX_KARCH) \
		-v $(LINUX_VERSION) \
		-s $(KDIR)/$(DEVICE_NAME)-u-boot-script \
		$(if $(FIT_KEY_NAME),-S $(FIT_KEY_NAME)) \
		$(if $(FW_AR_VER),-r $(FW_AR_VER))
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage \
		-f $@.its \
		$(if $(FIT_KEY_DIR),-k $(FIT_KEY_DIR)) \
		-r \
		$@.new
	@mv $@.new $@
endef

# default all platform image(fit) build 
define Device/Default
  PROFILES = Default $$(DEVICE_NAME)
  KERNEL_NAME := Image
  KERNEL = kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  KERNEL_INITRAMFS = kernel-bin | lzma | \
	fit lzma $$(KDIR)/image-$$(firstword $$(DEVICE_DTS)).dtb
  FILESYSTEMS := squashfs
  DEVICE_DTS_DIR := $(DTS_DIR)
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-kernel | pad-to 128k | append-rootfs | \
	pad-rootfs | append-metadata
  IMAGE_ROOTFS := $(KDIR)/root.squashfs
  FIT_KEY_DIR :=
  FIT_KEY_NAME :=
endef

include $(SUBTARGET).mk

define Image/Build
	$(call Image/Build/$(1),$(1))
ifeq ($(CONFIG_ACEN_ENCRYPTED_IMAGE), y)
	openssl enc -aes-256-cbc -in $(BIN_DIR)/openwrt-mediatek-ex5601t0-ex5601-t0-squashfs-sysupgrade.bin -out $(BIN_DIR)/ras_encrypted.tmp -pass pass:$(shell $(ZYXEL_PRIVATE_PACKAGE)/zyw88/zywkb)
	wc -c $(BIN_DIR)/ras_encrypted.tmp | cut -d " " -f1
	echo "<encrypted>" > $(BIN_DIR)/ras_encrypted_header
	cat $(BIN_DIR)/ras_encrypted_header $(BIN_DIR)/ras_encrypted.tmp > $(BIN_DIR)/openwrt-mediatek-ex5601t0-ex5601-t0-squashfs-sysupgrade_encrypted.bin
	rm $(BIN_DIR)/ras_encrypted_header $(BIN_DIR)/ras_encrypted.tmp
endif
endef

define Image/Prepare
	cp `find $(LINUX_DIR)/ -name "*.ko"` $(TARGET_DIR)/lib/modules/$(LINUX_VERSION)/
	$(call Image/Prepare/Default)
endef

$(eval $(call BuildImage))



# copy from include/image-commands.mk

ZYBUILD_DATE = $(if $(wildcard $(KDIR)/tmp/build_date_$(DEVICE_NAME)),$(shell cat $(KDIR)/tmp/build_date_$(DEVICE_NAME)),`date +%m/%d/%Y`)
ZYBUILD_TIME = $(if $(wildcard $(KDIR)/tmp/build_time_$(DEVICE_NAME)),$(shell cat $(KDIR)/tmp/build_time_$(DEVICE_NAME)),`date +%02k:%02M:%02S`)

ZYXEL_FIRMWARE_VERSION=$(if $(call qstrip,$(CONFIG_ZYXEL_FIRMWARE_VERSION)),$(CONFIG_ZYXEL_FIRMWARE_VERSION),"V0.00(AAA.0)b0")
ifeq ($(call qstrip,$(CONFIG_ZYXEL_FIRMWARE_VERSION_EXT)),)
ZYXEL_FIRMWARE_VERSION_EXT = $(ZYXEL_FIRMWARE_VERSION)
else
ZYXEL_FIRMWARE_VERSION_EXT = $(CONFIG_ZYXEL_FIRMWARE_VERSION_EXT)
endif

# reference metadata_json
zymetadata_json = \
	'{ $(if $(IMAGE_METADATA),$(IMAGE_METADATA)$(comma)) \
		"metadata_version": "1.1", \
		"compat_version": "$(call json_quote,$(compat_version))", \
		$(if $(DEVICE_COMPAT_MESSAGE),"compat_message": "$(call json_quote,$(DEVICE_COMPAT_MESSAGE))"$(comma)) \
		$(if $(filter-out 1.0,$(compat_version)),"new_supported_devices": \
			[$(call metadata_devices,$(SUPPORTED_DEVICES))]$(comma) \
			"supported_devices": ["$(call json_quote,$(legacy_supported_message))"]$(comma)) \
		$(if $(filter 1.0,$(compat_version)),"supported_devices":[$(call metadata_devices,$(SUPPORTED_DEVICES))]$(comma)) \
		"version": { \
			"dist": "$(call json_quote,$(VERSION_DIST))", \
			"version": "$(call json_quote,$(VERSION_NUMBER))", \
			"revision": "$(call json_quote,$(REVISION))", \
			"target": "$(call json_quote,$(TARGETID))", \
			"board": "$(call json_quote,$(if $(BOARD_NAME),$(BOARD_NAME),$(DEVICE_NAME)))" \
		}, \
		"zyfwinfo": { \
			"product_name": $(if $(call qstrip,$(CONFIG_MRD_PRODUCT_NAME)),$(CONFIG_MRD_PRODUCT_NAME),"WX5600-T0"), \
			"model_id": $(if $(call qstrip,$(CONFIG_MRD_MODEL_ID)),$(CONFIG_MRD_MODEL_ID),"4A00"), \
			"fw_ver": $(ZYXEL_FIRMWARE_VERSION), \
			"extfw_ver": $(ZYXEL_FIRMWARE_VERSION_EXT), \
			"build_date": "$(shell echo $(ZYBUILD_DATE))", \
			"build_time": "$(shell echo $(ZYBUILD_TIME))", \
			"kernel_chksum": "$(shell zychksum $(IMAGE_KERNEL) $(if $(CONFIG_BIG_ENDIAN),,little))", \
			"rootfs_chksum": "$(shell zychksum $(IMAGE_ROOTFS) $(if $(CONFIG_BIG_ENDIAN),,little))", \
			"blocksize": $(if $(CONFIG_TARGET_SQUASHFS_BLOCK_SIZE),"$(CONFIG_TARGET_SQUASHFS_BLOCK_SIZE)kB","UNKNOW") \
		} \
	}'

# reference Build/append-metadata
define Build/append-zymetadata
	$(if $(SUPPORTED_DEVICES),-echo $(call zymetadata_json) | fwtool -I - $@)
	[ ! -s "$(BUILD_KEY)" -o ! -s "$(BUILD_KEY).ucert" -o ! -s "$@" ] || { \
		cp "$(BUILD_KEY).ucert" "$@.ucert" ;\
		usign -S -m "$@" -s "$(BUILD_KEY)" -x "$@.sig" ;\
		ucert -A -c "$@.ucert" -x "$@.sig" ;\
		fwtool -S "$@.ucert" "$@" ;\
	}
endef

# reference Build/sysupgrade-tar
define Build/sysupgrade-zytar
	echo zyfwinfo=$(KDIR)/tmp/$(IMG_PREFIX)-$(DEVICE_NAME)-$(FILESYSTEM)-zyfwinfo.bin
	echo zydefault=$(KDIR)/tmp/$(IMG_PREFIX)-$(DEVICE_NAME)-$(FILESYSTEM)-zydefault.bin
	sh $(TOPDIR)/scripts/sysupgrade-zytar.sh \
		--board $(if $(BOARD_NAME),$(BOARD_NAME),$(DEVICE_NAME)) \
		--kernel $(call param_get_default,kernel,$(1),$(IMAGE_KERNEL)) \
		--rootfs $(call param_get_default,rootfs,$(1),$(IMAGE_ROOTFS)) \
		--zyfwinfo $(KDIR)/tmp/$(IMG_PREFIX)-$(DEVICE_NAME)-$(FILESYSTEM)-zyfwinfo.bin \
		--zydefault $(KDIR)/tmp/$(IMG_PREFIX)-$(DEVICE_NAME)-$(FILESYSTEM)-zydefault.bin \
		$@
endef

#save at $(KDIR)/tmp/$(IMG_PREFIX)-$(DEVICE_NAME)-$(FILESYSTEM)-zyfwinfo.bin
define Build/gen-zyfwinfo
	date +%m/%d/%Y > $(KDIR)/tmp/build_date_$(DEVICE_NAME)
	date +%02k:%02M:%02S > $(KDIR)/tmp/build_time_$(DEVICE_NAME)
	$(STAGING_DIR_HOST)/bin/zyfwinfo \
		$(if $(CONFIG_BIG_ENDIAN),,-e little) \
		-n 0 \
		-d $(shell echo $(ZYBUILD_DATE)) \
		-t $(shell echo $(ZYBUILD_TIME)) \
		-k $(call param_get_default,kernel,$(1),$(IMAGE_KERNEL)) \
		-r $(call param_get_default,rootfs,$(1),$(IMAGE_ROOTFS)) \
		-s $(ZYXEL_FIRMWARE_VERSION) \
		-S $(ZYXEL_FIRMWARE_VERSION_EXT) \
		-p $(if $(call qstrip,$(CONFIG_MRD_PRODUCT_NAME)),$(CONFIG_MRD_PRODUCT_NAME),"WX5600-T0") \
		-m $(if $(call qstrip,$(CONFIG_MRD_MODEL_ID)),$(CONFIG_MRD_MODEL_ID),"4A00") \
		-o $@
endef

define Build/gen-zydefault
	@echo "process Build/gen-zydefault"
	mkdir -p $(KDIR)/zydefault
	cp $(STAGING_DIR)/usr/zydefault/* $(KDIR)/zydefault

	$(STAGING_DIR_HOST)/bin/mksquashfs4 \
		$(KDIR)/zydefault \
		$@ \
		-nopad -noappend -root-owned \
		-comp $(SQUASHFSCOMP) -b $(SQUASHFS_BLOCKSIZE)
endef

# reference Build/append-ubi
define Build/append-zyubi
	sh $(TOPDIR)/scripts/ubinize-zyimage.sh \
		$(if $(UBOOTENV_IN_UBI),--uboot-env) \
		$(if $(KERNEL_IN_UBI),--kernel $(IMAGE_KERNEL)) \
		--zyfwinfo $(KDIR)/tmp/$(IMG_PREFIX)-$(DEVICE_NAME)-$(FILESYSTEM)-zyfwinfo.bin \
		--zydefault $(KDIR)/tmp/$(IMG_PREFIX)-$(DEVICE_NAME)-$(FILESYSTEM)-zydefault.bin \
		$(foreach part,$(UBINIZE_PARTS),--part $(part)) \
		$(IMAGE_ROOTFS) \
		$@.tmp \
		-p $(BLOCKSIZE:%k=%KiB) -m $(PAGESIZE) \
		$(if $(SUBPAGESIZE),-s $(SUBPAGESIZE)) \
		$(if $(VID_HDR_OFFSET),-O $(VID_HDR_OFFSET)) \
		$(UBINIZE_OPTS)
	cat $@.tmp >> $@
	rm $@.tmp
endef

zcfg_project_name=$(shell (grep CONFIG_ZCFG_PROJECT $(TOPDIR)/.config | awk -F"=" '{print $$2}' | sed "s/\"//g"))
check_TE_config=$(TOPDIR)/package/private/zyxel/zcfg/defcfg/$(zcfg_project_name)/TE_config.rom
# ps: ENDIAN 1:Little endian 0:Big endian
define Build/append-romfileubi
	if [ -f $(check_TE_config) ]; then \
		cp $(check_TE_config) $(TOPDIR)/tmp/TE_config_tmp.rom; \
		sh $(TOPDIR)/scripts/ubinize-romfile.sh \
			--romfile $(TOPDIR)/tmp/TE_config_tmp.rom \
			--tmp_path $(TOPDIR)/tmp \
			--vol_size 0x00200000 \
			--ENDIAN Little \
			$@.tmp \
			-p $(BLOCKSIZE:%k=%KiB) -m $(PAGESIZE) \
			$(if $(SUBPAGESIZE),-s $(SUBPAGESIZE)) \
			$(if $(VID_HDR_OFFSET),-O $(VID_HDR_OFFSET)) \
			$(UBINIZE_OPTS); \
		cat $@.tmp >> $@; \
		rm $@.tmp; \
	else \
		echo "missing: $(check_TE_config)"; \
		touch $@; \
	fi
endef

define Build/append-zyubi-te
	if [ -f $(check_TE_config) ]; then \
		mkdir -p $(TOPDIR)/tmp/TE_config; \
		cp $(check_TE_config) $(TOPDIR)/tmp/TE_config/TE_config.rom; \
		$(STAGING_DIR_HOST)/bin/mksquashfs4 \
			$(TOPDIR)/tmp/TE_config \
			$(TOPDIR)/tmp/TE_config.rom.squashfs \
			-nopad -noappend -root-owned \
			-comp $(SQUASHFSCOMP) -b $(SQUASHFS_BLOCKSIZE); \
		sh $(TOPDIR)/scripts/ubinize-zyimage.sh \
			$(if $(UBOOTENV_IN_UBI),--uboot-env) \
			$(if $(KERNEL_IN_UBI),--kernel $(IMAGE_KERNEL)) \
			--zyfwinfo $(KDIR)/tmp/$(IMG_PREFIX)-$(DEVICE_NAME)-$(FILESYSTEM)-zyfwinfo.bin \
			--zydefault $(KDIR)/tmp/$(IMG_PREFIX)-$(DEVICE_NAME)-$(FILESYSTEM)-zydefault.bin \
			--teconfig $(TOPDIR)/tmp/TE_config.rom.squashfs \
			$(foreach part,$(UBINIZE_PARTS),--part $(part)) \
			$(IMAGE_ROOTFS) \
			$@.tmp \
			-p $(BLOCKSIZE:%k=%KiB) -m $(PAGESIZE) \
			$(if $(SUBPAGESIZE),-s $(SUBPAGESIZE)) \
			$(if $(VID_HDR_OFFSET),-O $(VID_HDR_OFFSET)) \
			$(UBINIZE_OPTS); \
		cat $@.tmp >> $@; \
		rm $@.tmp; \
		rm -rf $(TOPDIR)/tmp/TE_config; \
	fi
endef

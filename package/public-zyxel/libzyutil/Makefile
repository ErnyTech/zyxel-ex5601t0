# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

#These lines concatanate the package name and list the URL location from which the package source code is to be downloaded
PKG_NAME:=libzyutil
PKG_VERSION:=2.0
PKG_RELEASE:=1

PKG_NAME_VER  := $(PKG_NAME)-$(PKG_VERSION)
SDK_VER=$(call qstrip,$(CONFIG_BRCM_SDK_VERSION))
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=http://www.cs.uh.edu/~vivekian/downloads/
#PKG_SOURCE_URL:=@ZyXEL_SITE/public/ZyXEL
DL_DIR:=$(TOPDIR)/dl/public/zyxel

PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

ifeq ($(CONFIG_ZYXEL_OPENWRT_14),y)
LIBJSON=libjson
else
LIBJSON=libjson-c
endif

#These lines describe what your package does and its place in the menu config
PKG_BUILD_DEPENDS:=zcfg $(LIBJSON) libopenssl libzlog libzos libzcfg_msg
define Package/libzyutil
SECTION:=net
CATEGORY:=Zyxel Packages
TITLE:= ZyXEL Configuration Management Utility Library
DESCRIPTION:=
#DEPENDS:=zcfg +libjson
endef

ifeq ($(CONFIG_EN7561SDK_ZYXEL_MULTI_PROVINCE_VENDOR),y)
$(shell cp $(PLATFORM_DIR)/$(SUBTARGET)/zyxel_province.h $(STAGING_DIR)/usr/include/)
$(shell cp $(PLATFORM_DIR)/$(SUBTARGET)/zyxel_province_struct.h $(STAGING_DIR)/usr/include/)
endif

#Disable Dependencies Check
define CheckDependencies
endef


ifeq ($(SDK_VER),504L02)
ZYXEL_LIB_CFLAGS += -D_DEFAULT_SOURCE
TOOLCHAIN_DIR=$(subst .,,$(CONFIG_TOOLCHAIN_INC_PATH))
ZYXEL_LIB_CFLAGS += -I$(CONFIG_TOOLCHAIN_ROOT)$(TOOLCHAIN_DIR)
else ifeq ("$(CONFIG_EXTERNAL_KERNEL_TOOLCHAIN)","")
ZYXEL_LIB_CFLAGS += -I$(LINUX_DIR)/user_headers/include
else
ZYXEL_LIB_CFLAGS += -I$(TOOLCHAIN_DIR)/include
endif

# Zyxel Feature Bits
ifeq ($(CONFIG_ZYXEL_FEATURE_BITS),y)
ZYXEL_LIB_CFLAGS += -I$(LINUX_DIR)/include/uapi
endif

ifeq ("$(strip ${ZCFG_PLATFORM})","ECONET")
ifneq ($($(shell echo CONFIG_$(CONFIG_CUSTOM_SDK_PREFIX)_ZYXEL_LAN_WAN_WLAN_SHARE_THE_SAME_MAC)),)
ZYXEL_LIB_CFLAGS += -DZYXEL_LAN_WAN_WLAN_SHARE_THE_SAME_MAC
endif
ifeq ($(CONFIG_TC_SDK_7_3_245_300),y)
ZYXEL_LIB_CFLAGS += -DNEW_MTD_DRIVER
endif
ifeq ($(CONFIG_TC_SDK_7_3_257_150),y)
ZYXEL_LIB_CFLAGS += -DNEW_MTD_DRIVER
endif
ifeq ($(CONFIG_TC_SDK_7_3_267_0),y)
ZYXEL_LIB_CFLAGS += -DNEW_MTD_DRIVER
endif
ifeq ($(CONFIG_TC_SDK_7_3_267_750),y)
ZYXEL_LIB_CFLAGS += -DNEW_MTD_DRIVER
endif
ifeq ($(CONFIG_TC_SDK_7_3_267_700),y)
ZYXEL_LIB_CFLAGS += -DNEW_MTD_DRIVER
endif
ifeq ($(CONFIG_ZYXEL_VMG8825_T50K_TAAAD_ISOLATE_DMZ),y)
ZYXEL_LIB_CFLAGS += -DZYXEL_VMG8825_T50K_TAAAD_ISOLATE_DMZ
endif
ifneq ($(strip $(TCSUPPORT_WLAN_MT7916D)),)
ZYXEL_LIB_CFLAGS += -DMT7916D
endif
ifeq ($(CONFIG_TC_SDK_7_3_251_900),y)
ZYXEL_LIB_CFLAGS += -DTC_SDK_7_3_251_900
endif

endif

ifeq ($(CONFIG_$(shell echo $(CONFIG_CUSTOM_SDK_PREFIX))_TCSUPPORT_LITTLE_ENDIAN),y)
ZYXEL_LIB_CFLAGS += -DTCSUPPORT_LITTLE_ENDIAN
endif

export CONFIG_ZYXEL_TUTK_CLOUD_AGENT
export CONFIG_ZYXEL_AT_SET_TUTK_UID

ifeq ($(CONFIG_BUILD_ZYXEL_VOICE_MTK_D2_VE_SUPPORT),y)
ZYXEL_LIB_CFLAGS += -DBUILD_ZYXEL_VOICE_MTK_D2_VE_SUPPORT
endif

# because Broadcom platform include make.common and it will overwrite CFLAGS
# We use another variable and append it after include make.common
MAKE_FLAGS += ZYXEL_LIB_CFLAGS="$(ZYXEL_LIB_CFLAGS)"
MAKE_FLAGS += PRODUCT_NAME="$(call qstrip, $(CONFIG_MRD_PRODUCT_NAME))"
TARGET_LDFLAGS += $(ZYXEL_LIB_LDFLAGS)
ifeq ("$(strip ${ZCFG_PLATFORM})","BROADCOM")
MAKE_FLAGS += "BCM_BUILD_DIR=$(BCM_BUILD_DIR)"
endif

# to include zyxel_province.h
ifeq ($(CONFIG_EN7561SDK_ZYXEL_MULTI_PROVINCE_VENDOR),y)
MAKE_FLAGS += ZYXEL_MULTI_PROVINCE_VENDOR="$(call qstrip, $(CONFIG_EN7561SDK_ZYXEL_MULTI_PROVINCE_VENDOR))"
endif

ifeq ($(CONFIG_ZYXEL_15_BYTE_SERIAL_NUMBER),y)
MAKE_FLAGS += ZYXEL_15_BYTE_SERIAL_NUMBER=$(CONFIG_ZYXEL_15_BYTE_SERIAL_NUMBER) 
endif

ifeq ($(CONFIG_ZYXEL_SYSTEMCMD_REPLACEMENT),y)
MAKE_FLAGS += ZYXEL_SYSTEMCMD_REPLACEMENT=$(CONFIG_ZYXEL_SYSTEMCMD_REPLACEMENT) 
endif

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	$$(call link_files,$(PKG_NAME_VER),$(BUILD_DIR))
ifeq ($(CONFIG_ZYXEL_OPENWRT_19),y)
	$(CP) $(STAGING_DIR_HOST)/usr/include/zyfwinfo.h $(PKG_BUILD_DIR)/BROADCOM
endif
ifeq ($(CONFIG_ZYXEL_OPENWRT_21),y)
	$(CP) $(STAGING_DIR_HOST)/usr/include/zyfwinfo.h $(PKG_BUILD_DIR)/MTK
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/libzyutil_encrypt/*.h $(1)/usr/include/
ifeq ($(CONFIG_ZYXEL_OPENWRT_19),y)
	$(CP) $(PKG_BUILD_DIR)/BROADCOM/zyfwinfo.h $(1)/usr/include/
endif
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libzyutil.so $(1)/usr/lib/
ifeq ($(CONFIG_ZYXEL_OPENWRT_19),y)
	$(CP) $(PKG_BUILD_DIR)/libdebug.a $(1)/usr/lib
endif
endef

define Package/libzyutil/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME).so $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libdebug.a $(STAGING_DIR)/usr/lib
#	$(INSTALL_DIR) $(1)/lib/public
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libzyutil.so $(1)/lib/public
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libdebug.a $(STAGING_DIR)/usr/lib
#	ln -s /lib/public/libzyutil.so $(1)/lib/libzyutil.so
endef

$(eval $(call BuildPackage,libzyutil))

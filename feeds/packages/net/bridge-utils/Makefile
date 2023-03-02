#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=bridge-utils
PKG_VERSION:=1.7
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
#PKG_SOURCE_URL:=@KERNEL/linux/kernel/people/shemminger/bridge-utils
#PKG_HASH:=939987e659b1a4c36ae46f44b6687f373bc5c916a9eab91f775630f5e38b997e
PKG_SOURCE_URL:=@ZyXEL_SITE/opensource/network/utils
PATCH_DIR:=patches/$(ZCFG_PLATFORM)

PKG_MAINTAINER:=Nikolay Martynov <mar.kolya@gmail.com>
PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/bridge
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=Ethernet bridging configuration utility
  URL:=http://www.linuxfromscratch.org/blfs/view/svn/basicnet/bridge-utils.html
  ALTERNATIVES:=300:/usr/sbin/brctl:/usr/libexec/brctl-bridge-utils
endef

define Package/bridge/description
 Manage ethernet bridging: a way to connect networks together to
 form a larger network.
endef

# add ECONET as reference
ifeq ("$(strip ${ZCFG_PLATFORM})","ECONET")
ifneq ($(strip $(CONFIG_$(subst $\",,$(CONFIG_CUSTOM_SDK_PREFIX))_TCSUPPORT_IGMP_SNOOPING)),)
ZCFLAGS += -DTCSUPPORT_IGMP_SNOOPING -DCONFIG_IGMP_SNOOPING -DCONFIG_BRIDGE_IGMP_SNOOPING
endif
ifneq ($(strip $(CONFIG_$(subst $\",,$(CONFIG_CUSTOM_SDK_PREFIX))_TCSUPPORT_IGMP_SNOOPING_V3)),)
ZCFLAGS += -DTCSUPPORT_IGMP_SNOOPING_V3
endif
ifneq ($(strip $(CONFIG_$(subst $\",,$(CONFIG_CUSTOM_SDK_PREFIX))_TCSUPPORT_IGMPSNOOPING_ENHANCE)),)
ZCFLAGS += -DTCSUPPORT_IGMPSNOOPING_ENHANCE
endif
ifneq ($(strip $(CONFIG_$(subst $\",,$(CONFIG_CUSTOM_SDK_PREFIX))_TCSUPPORT_MLD_SNOOPING)),)
ZCFLAGS += -DTCSUPPORT_MLD_SNOOPING -DCONFIG_MLD_SNOOPING
endif
ifneq ($(strip $(CONFIG_$(subst $\",,$(CONFIG_CUSTOM_SDK_PREFIX))_TCSUPPORT_OMCI)),)
ZCFLAGS += -DTCSUPPORT_OMCI
endif
endif

ZCFLAGS += -DPLATFORM_$(ZCFG_PLATFORM)


ifneq ($(strip $(CONFIG_PACKAGE_zcfg)),)
ZCFLAGS += -DZCFG_USED -DZYXEL_PATCH
endif

TARGET_CFLAGS += $(ZCFLAGS)

CONFIGURE_ARGS += \
	--with-linux-headers="$(LINUX_DIR)"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/libbridge/libbridge.h $(1)/usr/include/
#ifneq ($(CONFIG_TARGET_mt75xx),)
ifeq ("$(strip ${ZCFG_PLATFORM})","ECONET")
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libbridge/libbridge.a $(1)/usr/lib
endif
endef

define Package/bridge/install
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/brctl $(1)/usr/libexec/brctl-bridge-utils
endef

$(eval $(call BuildPackage,bridge))

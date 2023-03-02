#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=busybox
CONFIG_busybox_revision="1.31.1"

ifeq ("$(strip ${ZCFG_PLATFORM})","QUALCOMM")
PKG_VERSION:=1.25.1
PKG_RELEASE:=2
PKG_FLAGS:=essential

#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE:=$(PKG_NAME)-$(PKG_V`ERSION).tar.bz2
else ifeq ($(CONFIG_busybox_revision), "1_20_1")
PKG_VERSION:=1_20_1
PKG_RELEASE:=5
PKG_FLAGS:=essential
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
else ifeq ($(CONFIG_busybox_revision), "1.31.1")
PKG_VERSION:=1.31.1
PKG_RELEASE:=1
PKG_FLAGS:=essential
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
else
#default
PKG_VERSION:=1_20_1
PKG_RELEASE:=5
PKG_FLAGS:=essential

#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
endif

#PKG_HASH:=3d1d04a4dbd34048f4794815a5c48ebb9eb53c5277e09ffffc060323b95dfbdc
PKG_SOURCE_URL:=@ZyXEL_SITE/opensource/utils/busybox

FILES_DIR:=files-$(PKG_VERSION)

PKG_BUILD_DEPENDS:=BUSYBOX_CONFIG_FEATURE_HAVE_RPC:librpc
PKG_BUILD_DEPENDS:=BUSYBOX_CONFIG_PAM:libpam
PKG_BUILD_DEPENDS+=PACKAGE_zcmd:zcfg PACKAGE_zcmd:libzcfg_msg PACKAGE_zcmd:libjson PACKAGE_zcmd:libzcfg_fe_rdm_access PACKAGE_zcmd:libzcmd_tool PACKAGE_zcmd:libzcfg_fe_rdm_struct PACKAGE_zcmd:libzyutil
#PKG_BUILD_DEPENDS+=PACKAGE_zcmd:libzcfg_msg PACKAGE_zcmd:libjson PACKAGE_zcmd:libzcfg_fe_rdm_access PACKAGE_zcmd:libzcmd_tool PACKAGE_zcmd:libzcfg_fe_rdm_struct
PKG_BUILD_PARALLEL:=1
PKG_CHECK_FORMAT_SECURITY:=0

PATCH_DIR:=patches-$(PKG_VERSION)

#Busybox use it's own PIE config flag and LDFLAGS are used with ld, not gcc.
PKG_ASLR_PIE:=0

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE archival/libarchive/bz/LICENSE
PKG_CPE_ID:=cpe:/a:busybox:busybox

include $(INCLUDE_DIR)/package.mk

ifeq ($(DUMP),)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell grep '^CONFIG_BUSYBOX_' $(TOPDIR)/.config | mkhash md5)
endif

BUSYBOX_SYM=$(if $(CONFIG_BUSYBOX_CUSTOM),CONFIG,DEFAULT)
BUSYBOX_IF_ENABLED=$(if $(CONFIG_BUSYBOX_$(BUSYBOX_SYM)_$(1)),$(2))

ifneq ($(findstring c,$(OPENWRT_VERBOSE)),)
  BB_MAKE_VERBOSE := V=1
else
  BB_MAKE_VERBOSE :=
endif

init-y :=
init-$(CONFIG_BUSYBOX_CONFIG_CROND) += cron
init-$(CONFIG_BUSYBOX_CONFIG_TELNETD) += telnet


# All files provided by busybox will serve as fallback alternatives by opkg.
# There should be no need to enumerate ALTERNATIVES entries here
define Package/busybox
  SECTION:=base
  CATEGORY:=Base system
  MAINTAINER:=Felix Fietkau <nbd@nbd.name>
  TITLE:=Core utilities for embedded Linux
  URL:=http://busybox.net/
  DEPENDS:=+BUSYBOX_CONFIG_PAM:libpam +BUSYBOX_CONFIG_NTPD:jsonfilter
  MENU:=1
endef

define Package/busybox/config
	select PACKAGE_libzcfg_fe_rdm_struct if PACKAGE_zcfg
	select PACKAGE_libzcfg_msg if PACKAGE_zcfg
	select PACKAGE_libjson if PACKAGE_zcfg
	select PACKAGE_libzyutil if PACKAGE_zcfg
endef

#Disable Dependencies Check
define CheckDependencies
endef

define Package/busybox/description
 The Swiss Army Knife of embedded Linux.
 It slices, it dices, it makes Julian Fries.
endef

define Package/busybox/config
	source "$(SOURCE)/Config.in"
endef

ifdef CONFIG_BUSYBOX_CONFIG_FEATURE_SYSLOG
define Package/busybox/conffiles
/etc/syslog.conf
endef
endif

# don't create a version string containing the actual timestamp
export KCONFIG_NOTIMESTAMP=1


ifndef CONFIG_USE_MUSL
LDLIBS:=m crypt
endif

ifdef CONFIG_BUSYBOX_CONFIG_FEATURE_HAVE_RPC
  TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
  export LDFLAGS=$(TARGET_LDFLAGS)
  LDLIBS += rpc
endif

LDLIBS += $(call BUSYBOX_IF_ENABLED,PAM,pam pam_misc pthread)
ifeq ($(CONFIG_USE_GLIBC),y)
  LDLIBS += $(call BUSYBOX_IF_ENABLED,NSLOOKUP_OPENWRT,resolv)
endif

ifdef CONFIG_GCC_VERSION_LLVM
  TARGET_CFLAGS += -fnested-functions
endif

#ifeq ($(CONFIG_PACKAGE_zcmd),y)
TARGET_CFLAGS += -DZCFG_SUPPORT
LDLIBS += json-c zcfg_msg zcfg_fe_rdm_access zcmd_tool zcfg_fe_rdm_struct zos zlog zyutil crypto
#endif

ifeq ($(TCSUPPORT_WAN_GPON),y)
TARGET_CFLAGS += -DTCSUPPORT_WAN_GPON
endif

ifeq ($(TCSUPPORT_3_18_21_KERNEL),y)
TARGET_CFLAGS += -DTCSUPPORT_3_18_21_KERNEL
endif

ifneq ($(strip ${ZCFG_PLATFORM}),)
TARGET_CFLAGS += -DPLATFORM_$(ZCFG_PLATFORM)
endif

ifeq ($(CONFIG_ZYXEL_SAVE_LOG_TO_FLASH),y)
TARGET_CFLAGS += -DZYXEL_SAVE_LOG_TO_FLASH
endif

ifeq ($(CONFIG_ZYXEL_DL_DIAG_CALCULATION),y)
TARGET_CFLAGS += -DZYXEL_DL_DIAG_CALCULATION
endif

ifeq ($(CONFIG_CAAAB_CUSTOMIZATION),y)
TARGET_CFLAGS += -DCAAAB_CUSTOMIZATION
endif

ifeq ($(CONFIG_ZYXEL_NAT_LOG),y)
TARGET_CFLAGS += -DZYXEL_NAT_LOG
endif

ifeq ($(CONFIG_HAAAA_CUSTOMIZATION),y)
TARGET_CFLAGS += -DHAAAA_CUSTOMIZATION
endif

ifeq ($(CONFIG_ZYXEL_REMOTE_ACCESS_PRIVILEGE),y)
TARGET_CFLAGS += -DZYXEL_REMOTE_ACCESS_PRIVILEGE
endif

ifeq ($(CONFIG_ZYXEL_REMOTE_ACCOUNT_ACCESS_PRIVILEGE),y)
TARGET_CFLAGS += -DZYXEL_REMOTE_ACCOUNT_ACCESS_PRIVILEGE
endif

ifeq ($(CONFIG_ABUU_CUSTOMIZATION),y)
TARGET_CFLAGS += -DABUU_CUSTOMIZATION
endif

ifeq ($(CONFIG_ZYXEL_REMOMGMT_ONLY_HTTPS_ALLOWS_FOR_WLAN_CONNECT),y)
TARGET_CFLAGS += -DZYXEL_REMOMGMT_ONLY_HTTPS_ALLOWS_FOR_WLAN_CONNECT
endif

ifeq ($(CONFIG_ZYXEL_CLI_IDLE_TIMEOUT),y)
TARGET_CFLAGS += -DZYXEL_CLI_IDLE_TIMEOUT
endif

ifeq ($(CONFIG_ABOG_CUSTOMIZATION),y)
TARGET_CFLAGS += -DABOG_CUSTOMIZATION
endif

ifeq ($(CONFIG_MSTC_TAAAG_MULTI_USER),y)
TARGET_CFLAGS += -DMSTC_TAAAG_MULTI_USER
endif

ifeq ($(CONFIG_ZYXEL_CUSTOMIZATION_SYSLOG),y)
TARGET_CFLAGS += -DZYXEL_CUSTOMIZATION_SYSLOG
endif

ifeq ($(CONFIG_$(shell echo $(CONFIG_CUSTOM_SDK_PREFIX))_TCSUPPORT_BTN_CHECK),y)
TARGET_CFLAGS += -DTCSUPPORT_BTN_CHECK
endif

ifeq ($(CONFIG_$(shell echo $(CONFIG_CUSTOM_SDK_PREFIX))_ZYXEL_DIAGNOSTIC_SPEEDTEST),y)
TARGET_CFLAGS += -DZYXEL_DIAGNOSTIC_SPEEDTEST
endif

ifeq ($(CONFIG_TAAAB_CUSTOMIZATION_TR069),y)
TARGET_CFLAGS += -DCONFIG_TAAAB_CUSTOMIZATION_TR069
endif

ifeq ($(CONFIG_ZYXEL_TR143_DIAG_ENHANCEMENT),y)
TARGET_CFLAGS += -DZYXEL_TR143_DIAG_ENHANCEMENT
endif

MAKE_VARS :=
MAKE_FLAGS += \
	EXTRA_CFLAGS="$(TARGET_CFLAGS) $(TARGET_CPPFLAGS)" \
	EXTRA_LDFLAGS="$(TARGET_LDFLAGS)" \
	LDLIBS="$(LDLIBS)" \
	LD="$(TARGET_CC)" \
	SKIP_STRIP=y
ifneq ($(findstring c,$(OPENWRT_VERBOSE)),)
  MAKE_FLAGS += V=1
endif

define Build/Configure
	grep 'CONFIG_BUSYBOX_$(BUSYBOX_SYM)' $(TOPDIR)/.config | sed -e "s,\\(# \)\\?CONFIG_BUSYBOX_$(BUSYBOX_SYM)_\\(.*\\),\\1CONFIG_\\2,g" > $(PKG_BUILD_DIR)/.config
	yes 'n' | $(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS) oldconfig
endef

define Build/Compile
	$(call Build/Compile/Default, \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		CONFIG_EXTRA_LDLIBS="$(LDLIBS)" \
		all install \
	)
endef

define Package/busybox/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
ifeq ("$(strip ${ZCFG_PLATFORM})","QUALCOMM")
	$(INSTALL_BIN) ./$(FILES_DIR)/cron $(1)/etc/init.d/cron
	$(INSTALL_BIN) ./$(FILES_DIR)/telnet $(1)/etc/init.d/telnet
	$(INSTALL_BIN) ./$(FILES_DIR)/sysntpd $(1)/etc/init.d/sysntpd
	$(INSTALL_BIN) ./$(FILES_DIR)/ntpd-hotplug $(1)/usr/sbin/ntpd-hotplug
	$(INSTALL_BIN) ./$(FILES_DIR)/inetd $(1)/etc/init.d/inetd
	-rm -rf $(1)/lib64
ifneq ($(CONFIG_BUSYBOX_CONFIG_FEATURE_USE_INITTAB),)
	mkdir -p $(1)/etc/init.d
	$(CP) ./$(FILES_DIR)/etc/inittab $(1)/etc/inittab
	$(CP) ./$(FILES_DIR)/etc/init.d/rcS $(1)/etc/init.d/rcS
endif
else
	for tmp in $(init-y); do \
		$(INSTALL_BIN) ./$(FILES_DIR)/$$$$tmp $(1)/etc/init.d/$$$$tmp; \
	done
	-rm -rf $(1)/lib64
ifdef CONFIG_BUSYBOX_CONFIG_PING
	chmod 4777 $(1)/bin/ping
endif
endif
endef

$(eval $(call BuildPackage,busybox))

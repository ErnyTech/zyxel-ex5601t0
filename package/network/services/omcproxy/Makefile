#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#


include $(TOPDIR)/rules.mk

PKG_NAME:=omcproxy
PKG_RELEASE:=10

PKG_VERSION:=2018-12-14-722151f0
PKG_NAME_VER=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
#PKG_SOURCE_PROTO:=git
#PKG_SOURCE_URL=$(PROJECT_GIT)/project/omcproxy.git
#PKG_SOURCE_DATE:=2018-12-14
#PKG_SOURCE_VERSION:=722151f04348cf1b759613c087bced52fb45790a
#PKG_MIRROR_HASH:=4d218923c149e2dc9010b8932ea92ab7e06f30df06814ffedaba7e081f2d4640
PKG_MAINTAINER:=Steven Barth <cyrus@openwrt.org>
PKG_LICENSE:=Apache-2.0

PKG_SOURCE_URL:=@ZyXEL_SITE/opensource/network/services

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/omcproxy
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libubox +libubus +libjson-c +libzyutil
  TITLE:=IGMPv3 and MLDv2 Multicast Proxy
endef

define Package/omcproxy/conffiles
#/etc/config/omcproxy
endef

CMAKE_OPTIONS += -DWITH_LIBUBOX=1

ifeq ("$(strip ${ZCFG_PLATFORM})","MTK")
CMAKE_OPTIONS += -DZYXEL_OMCPROXY=1
TARGET_CFLAGS += -DZYXEL_OMCPROXY
endif

define Package/omcproxy/install
#	$(INSTALL_DIR) $(1)/etc/config
#	$(INSTALL_CONF) ./files/omcproxy.config $(1)/etc/config/omcproxy
#	$(INSTALL_DIR) $(1)/etc/init.d
#	$(INSTALL_BIN) ./files/omcproxy.init $(1)/etc/init.d/omcproxy
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/omcproxy $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/omcpmsgcl $(1)/usr/sbin/

	ln -s ./omcproxy $(1)/usr/sbin/mldproxy
endef

$(eval $(call BuildPackage,omcproxy))

#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=zebra
PKG_VERSION:=0.95a

PATCH_DIR:= patches

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:= \
	http://matt.ucc.asn.au/dropbear/releases/ \
	http://www.mirrors.wiretapped.net/security/cryptography/apps/ssh/dropbear/
#PKG_MD5SUM:=8c784baec3054cdb1bb4bfa792c87812
PKG_SOURCE_URL:=@ZyXEL_SITE/opensource/network/services

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/zebra
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=RIP daemon
endef

CONFIGURE_ARGS += \
		--disable-ipv6 \
		--disable-bgpd \
		--disable-ospfd \
		--disable-ospf6d \
		--disable-ripngd \
		--disable-bgp-announce

TARGET_CFLAGS += -O2 -Wall -Wno-format-security


define Package/zebra/install	
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/zebra/zebra $(1)/usr/bin/zebra
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ripd/ripd $(1)/usr/bin/ripd
endef


$(eval $(call BuildPackage,zebra))

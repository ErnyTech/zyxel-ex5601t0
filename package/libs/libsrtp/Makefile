#
# Copyright (C) 2006-2018 OpenWrt.org
# Copyright (C) 2011 Victor Seva <linuxmaniac@torreviejawireless.org>
# Copyright (C) 2017 - 2018 Jiri Slachta <jiri@slachta.eu>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=libsrtp
PKG_VERSION:=1.6.0
PKG_RELEASE:=2


#--------------------------------------------------------------------------------
ifeq ("$(strip $(PKG_VERSION))","1.4.2")

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
#PKG_SOURCE_URL:=http://oss.metaparadigm.com/json-c/
PKG_SOURCE_URL:=@ZyXEL_SITE/opensource/libs

PATCH_DIR:=patches-$(PKG_VERSION)

PKG_LICENSE:=BSD
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=David A. McGrew <mcgrew@cisco.com>

#PKG_FIXUP:=autoreconf
#PKG_INSTALL:=1

#--------------------------------------------------------------------------------
else ifeq ("$(strip $(PKG_VERSION))","1.6.0")

#PKG_SOURCE_PROTO:=git
#PKG_SOURCE_URL:=https://github.com/cisco/libsrtp.git
#PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE_VERSION:=2b091a4fb21c9b06fc5d4b5763bdaec97542fcd7
#PKG_MIRROR_HASH:=7ac6c3113fd7a966c9a9108c185d9425e75a8fdc600e4ccef158e68ee1988c89
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
#
#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

#................................................................................
ifeq ("$(strip $(PKG_RELEASE))","1")
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
#................................................................................
else ifeq ("$(strip $(PKG_RELEASE))","2")
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MIRROR_HASH:=7ac6c3113fd7a966c9a9108c185d9425e75a8fdc600e4ccef158e68ee1988c89
#................................................................................
else
#To-Be-Defined for $(PKG_RELEASE) >= 3!!
#................................................................................
endif
PKG_SOURCE_URL:=@ZyXEL_SITE/opensource/libs

PATCH_DIR:=patches-$(PKG_VERSION)-$(PKG_RELEASE)

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jiri Slachta <jiri@slachta.eu>

#--------------------------------------------------------------------------------
else

#To-Be-Defined for $(PKG_VERSION) > 1.6.0!!

#--------------------------------------------------------------------------------
endif


include $(INCLUDE_DIR)/package.mk

###Michael.20180319.001: Make the SRTP Library support to be well-compiled for the BRCM New SDK 502L04-based Image of the ARM-architecture Platform (BCM63138/63137/63136-based VMG8823-Bx0B) by NOT using the INLINE function(s).
### PLATFORM: BRCM (963XX).-------------------------------------------------------------------------------
define Convert_BRCM_SDK_VERSION_To_NUMBER
$(shell B=$$((`echo $(1) | cut -f 1 -d 'L'` / 100)); R=$$((`echo $(1) | cut -f 1 -d 'L'` % 100)); C=$$((`echo $(1) | cut -f 2 -d 'L' | sed 's/[^0-9]*//g' | cut -c 1-2`)); M=`echo $(1) | cut -f 2 -d 'L' | sed 's/[^A-Fa-f]*//g' | cut -c 1`; E=$$((`[ "$$M" != "" ] && printf "%d" "0x$$M" || printf 0`)); V=$$(( $$((B << 20)) + $$((R << 12)) + $$((C << 4)) + $$((E)) )); echo $$V;)
endef

ifneq ("$(strip $(CONFIG_TARGET_brcm963xx))","")

	ifneq ("$(strip $(BRCM_SDK_VERSION))","")
		BRCM_SDK_VER_NUM=$(call Convert_BRCM_SDK_VERSION_To_NUMBER, $(BRCM_SDK_VERSION))
	else
		BRCM_SDK_VER_NUM=$(call Convert_BRCM_SDK_VERSION_To_NUMBER, $(CONFIG_BRCM_SDK_VERSION))
	endif

	BRCM_SDK_VER_NUM_500L01=$(call Convert_BRCM_SDK_VERSION_To_NUMBER, "500L01")

	ifeq ($(shell test $(BRCM_SDK_VER_NUM) -ge $(BRCM_SDK_VER_NUM_500L01); echo $$?),0)
		ifeq ($(CONFIG_ARCH), "mips")
			TARGET_CONFIGURE_OPTS += FUNC_INLINE=no
		else ifeq ($(CONFIG_arm), y)
			TARGET_CONFIGURE_OPTS += FUNC_INLINE=no
		endif
	endif
endif
### ------------------------------------------------------------------------------------------------------

CONFIGURE_ARGS += \
	--disable-stdout \
	--enable-syslog

define Package/libsrtp
  SUBMENU:=Telephony
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Secure Real-time Transport Protocol (SRTP) library
  DEPENDS:=+libpcap
  URL:=http://sourceforge.net/projects/srtp
endef

define Package/libsrtp/description
 Open-source implementation of the Secure Real-time Transport
 Protocol (SRTP) originally authored by Cisco Systems, Inc.
 It is available under a BSD-style license.
endef

define Build/Install
	$(MAKE) -C $(PKG_BUILD_DIR) DESTDIR=$(PKG_INSTALL_DIR) uninstall
	$(call Build/Install/Default)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
endef

### Michael.20150318.002: Modify to change the Voice/VoIP-related OpenWRT-based Common Reference Resources (Header file + Library) Install Path/Directory.
#define Build/InstallDev
#	$(INSTALL_DIR) $(1)/usr/include
#	$(CP) $(PKG_INSTALL_DIR)/usr/include/srtp $(1)/usr/include/
#	$(INSTALL_DIR) $(1)/usr/lib
#	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libsrtp.{a,so*} $(1)/usr/lib/
#	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
#	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/libsrtp.pc $(1)/usr/lib/pkgconfig/
#endef
define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/voice/srtp
	$(CP) $(PKG_BUILD_DIR)/include/*.h  $(STAGING_DIR)/usr/include/voice/srtp/

	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/voice/srtp/crypto
	$(CP) $(PKG_BUILD_DIR)/crypto/include/*.h  $(STAGING_DIR)/usr/include/voice/srtp/crypto/
endef


### Michael.20150318.002: Modify to change the Voice/VoIP-related OpenWRT-based Common Reference Resources (Header file + Library) Install Path/Directory.
#define Package/libsrtp/install
#	$(INSTALL_DIR) $(1)/usr/lib
#	$(CP) \
#		$(PKG_INSTALL_DIR)/usr/lib/libsrtp.so* \
#		$(1)/usr/lib/
#endef
# Amber.20201216: Add install target path.
define Package/libsrtp/install
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib/voice
	$(CP) $(PKG_BUILD_DIR)/libsrtp.a  $(STAGING_DIR)/usr/lib/voice/
endef

$(eval $(call BuildPackage,libsrtp))

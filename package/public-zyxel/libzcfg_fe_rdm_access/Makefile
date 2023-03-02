# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id$
	
include $(TOPDIR)/rules.mk

#These lines concatanate the package name and list the URL location from which the package source code is to be downloaded
PKG_NAME:=libzcfg_fe_rdm_access
PKG_VERSION:=2.0
PKG_RELEASE:=1

PKG_NAME_VER  := $(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=http://www.cs.uh.edu/~vivekian/downloads/
#PKG_SOURCE_URL:=@ZyXEL_SITE/public/ZyXEL
DL_DIR:=$(TOPDIR)/dl/public/zyxel

#md5 can be computed using the md5sum utility available on linux

PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

PKG_BUILD_DEPENDS:=zcfg libzcfg_msg libzcmd_tool

#These lines describe what your package does and its place in the menu config


export CONFIG_TAAAB_CUSTOMIZATION_TR069 CONFIG_ABUU_CUSTOMIZATION_MULTIPLE_CONFIG CONFIG_ZYXEL_WIFI_KEY_REMINDER CONFIG_ZYXEL_WEB_GUI_SHOW_ZYEE CONFIG_ZYXEL_PARTIAL_RESET

TARGET_CFLAGS += $(ZYXEL_LIB_CFLAGS)
TARGET_LDFLAGS += $(ZYXEL_LIB_LDFLAGS)

define Package/libzcfg_fe_rdm_access
SECTION:=net
CATEGORY:=Zyxel Packages
TITLE:= ZyXEL Configuration Management Frontend Data Model Access Library 
DESCRIPTION:=
#DEPENDS:=zcfg +libzcfg_msg +libzcmd_tool
endef

#Disable Dependencies Check
define CheckDependencies
endef

#For the compile rules you need to specify the cross compiler which needs to be used :


#These lines describe where your binary images are to be installed - its the equivalent to make install

# This particular package is a library and the installation rules reflect it

# Use openwrt default Build/Compile/Default
#define Package/libzcfg_fe_rdm_access/compile
#	$(MAKE) -C $(PKG_BUILD_DIR)
#endef

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	$$(call link_files,$(PKG_NAME_VER),$(BUILD_DIR))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libzcfg_fe_rdm_access.so $(1)/usr/lib/
endef

define Package/libzcfg_fe_rdm_access/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME).so $(1)/lib
#	$(INSTALL_DIR) $(1)/lib/public
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libzcfg_fe_rdm_access.so $(1)/lib/public
#	ln -s /lib/public/libzcfg_fe_rdm_access.so $(1)/lib/libzcfg_fe_rdm_access.so
endef

$(eval $(call BuildPackage,libzcfg_fe_rdm_access))


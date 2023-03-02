include $(TOPDIR)/rules.mk
#include $(INCLUDE_DIR)/opal_inc.mk

PKG_NAME:=libzcfg_dbg
PKG_VERSION:=2.0
PKG_RELEASE:=1

PKG_NAME_VER  := $(PKG_NAME)-$(PKG_VERSION)

#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=@ZyXEL_SITE/private/ZyXEL
PKG_SOURCE:=
PKG_SOURCE_URL:=
DL_DIR:=$(TOPDIR)/dl/public

PKG_CAT:=zcat

CURR_DIR=$(shell pwd)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)


PKG_SRC_DIR:=$(CURR_DIR)/src/$(PKG_NAME)-$(PKG_VERSION)

# PACKAGE_DIR:=$(BIN_DIR)/packages/private

include $(INCLUDE_DIR)/package.mk
PKG_BUILD_DEPENDS:=zcfg
define Package/libzcfg_dbg
  CATEGORY:= ZyXEL Packages
  TITLE:= ZyXEL Configuration Debugging library
  DESCRIPTION:=
#  DEPENDS:= zcfg
endef

#Disable Dependencies Check
#define CheckDependencies
#endef

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	$$(call link_files,$(PKG_NAME_VER),$(BUILD_DIR))
endef

define Build/Compile
	@echo "==> Build/Compile"
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CROSS)gcc" \
		STAGING_DIR=$(STAGING_DIR) \
		USRLIBDIR=$(STAGING_DIR)/usr/lib \
		USRINCDIR=$(STAGING_DIR)/usr/include \
		BUILD_DIR=$(BUILD_DIR) \
		PKG_BUILD_DIR="$(PKG_BUILD_DIR)" \
		INSTALL_DIR="$(PKG_INSTALL_DIR)"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/be_module_debug.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libzcfg_dbg.so $(1)/usr/lib/
endef

define Package/libzcfg_dbg/install
	@echo "==> Package/libzcfg_dbg/install"
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/lib/public
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libzcfg_dbg.so $(1)/lib/public
	ln -s /lib/public/libzcfg_dbg.so $(1)/lib/libzcfg_dbg.so
endef

define Build/Clean
	@echo "==> Build/Clean"
	-$(MAKE) -C $(PKG_BUILD_DIR) clean
endef

$(eval $(call BuildPackage,libzcfg_dbg))


include $(TOPDIR)/rules.mk

PKG_NAME:=libztr69_common
PKG_VERSION:=1.0
PKG_RELEASE:=1

CURR_DIR=$(shell pwd)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_NAME_VER  := $(PKG_NAME)-$(PKG_VERSION)

PKG_SRC_DIR:=$(CURR_DIR)/src/$(PKG_NAME)-$(PKG_VERSION)

CLI_APPS:=$(PKG_BUILD_DIR)/ztr69cli

# PACKAGE_DIR:=$(BIN_DIR)/packages/private

ifeq ($(CONFIG_ZYXEL_ZTR69_CLI),y)
export CONFIG_ZYXEL_ZTR69_CLI
endif

include $(INCLUDE_DIR)/package.mk
PKG_BUILD_DEPENDS:=ztr69
define Package/libztr69_common
  CATEGORY:= ZyXEL Packages
  TITLE:= ZyXEL TR69 Command Line Interface
#  DESCRIPTION:= ZyXEL TR69 Command Line Interface
#  DEPENDS:= ztr69
  DEPENDS:=+libpthread +clinkc +openssl-util +libuuid +
endef

#Disable Dependencies Check
define CheckDependencies
endef

define Package/libztr69_common/config
  
endef

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
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libztr69_common.so $(1)/usr/lib
endef

define Package/libztr69_common/install
	@echo "==> Package/libztr69_common/install"
	$(INSTALL_DIR) $(1)/bin
ifeq ($(CONFIG_ZYXEL_ZTR69_CLI),y)
	$(CP) $(CLI_APPS) $(1)/bin
endif
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_BUILD_DIR)/libztr69_common.so $(1)/lib
endef

define Build/Clean
	@echo "==> Build/Clean"
	-$(MAKE) -C $(PKG_BUILD_DIR) clean
endef

$(eval $(call BuildPackage,libztr69_common))


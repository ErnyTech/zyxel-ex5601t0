include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=libzyuitimer
PKG_VERSION:=2.0
PKG_RELEASE:=1

PKG_NAME_VER  := $(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=@ZyXEL_SITE/public/ZyXEL
DL_DIR:=$(TOPDIR)/dl/public/zyxel

PKG_CAT:=zcat

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

PKG_BUILD_DEPENDS:=

define Package/libzyuitimer
SECTION:=net
CATEGORY:=Zyxel Packages
TITLE:= ZyXEL User Interface Timer
DESCRIPTION:=
DEPENDS:=+libpthread +libzos +libzlog
endef

ifeq ($(CONFIG_USE_MUSL),y)
export LIBC_USE_MUSL=y
endif

#Disable Dependencies Check
define CheckDependencies
endef

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	$$(call link_files,$(PKG_NAME_VER),$(BUILD_DIR))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libzyuitimer.so $(1)/usr/lib/
endef

define Package/libzyuitimer/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME).so $(1)/lib
endef

$(eval $(call BuildPackage,libzyuitimer))

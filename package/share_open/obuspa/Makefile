include $(TOPDIR)/rules.mk

PKG_NAME:=obuspa
PKG_VERSION:=5.0.0
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_RELEASE:=1

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE

LOCAL_SRC:=$(TOPDIR)/package/share_open/$(PKG_NAME)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/local-development.mk
include $(INCLUDE_DIR)/package.mk

ifeq ($(CONFIG_PACKAGE_libwebsockets),y)
CONFIGURE_ARGS +=libwebsockets_CFLAGS="-I$(STAGING_DIR)/usr/include/ -I$(STAGING_DIR)/include"
CONFIGURE_ARGS +=libwebsockets_LIBS="-L$(STAGING_DIR)/usr/lib -lwebsockets"
else
CONFIGURE_ARGS += --disable-websockets
endif
 
ifeq ($(CONFIG_PACKAGE_libmosquitto),y)
else
CONFIGURE_ARGS += --disable-mqtt
endif		

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Zyxel share open packages
  TITLE:=USP(TR369) agent
  DEPENDS:=+libopenssl +libsqlite3 +libcurl PACKAGE_libwebsockets:libwebsockets PACKAGE_libmosquitto:libmosquitto +libjson-c +libzcfg_fe_rdm_string +libzcfg_msg +libzcfg_fe_schema zcfg
endef
# Add Zyxel's library 
CONFIGURE_ARGS += LIBS="-lzcfg_fe_schema -lzcfg_fe_rdm_string -lzcfg_msg -ljson-c"

PROJECT_NANE := $(strip $(shell cat $(TOPDIR)/.last_project 2>/dev/null))

define Build/Prepare
	$(call Build/Prepare/Default)
	@if test -f ./customer/$(PROJECT_NANE)/vendor_customize.am; then\
		echo "PROJECT_NANE=$(PROJECT_NANE) exists" ;\
		cp -f ./customer/$(PROJECT_NANE)/tr181_customize/*.c $(PKG_BUILD_DIR)/src/vendor/zyxel/tr181_customize/ ;\
		cp -f ./customer/$(PROJECT_NANE)/vendor_customize.am $(PKG_BUILD_DIR)/src/vendor/zyxel/ ;\
		cp -f ./customer/$(PROJECT_NANE)/mqtt.txt $(PKG_BUILD_DIR)/src/vendor/zyxel/ ;\
	else \
		echo "PROJECT_NANE=$(PROJECT_NANE) not exist" ;\
	fi

endef

define Package/$(PKG_NAME)/description
  OB-USP-AGENT is a system daemon providing a User Services Platform (USP) Agent.
endef

define Package/$(PKG_NAME)/install
  	$(INSTALL_DIR) $(1)/var/obuspa/

	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/obuspa
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/obuspa $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/vendor/zyxel/mqtt.txt $(1)/etc/obuspa/mqtt.txt
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
